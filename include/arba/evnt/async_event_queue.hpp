#pragma once

#include "event_manager.hpp"
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <memory>

inline namespace arba
{
namespace evnt
{

class async_event_queue
{
private:
    class async_event_queue_interface
    {
    public:
        virtual ~async_event_queue_interface() = default;
        virtual void emit(event_manager& evt_manager) = 0;
        virtual void sync() = 0;
    };
    using async_event_queue_interface_uptr = std::unique_ptr<async_event_queue_interface>;

    template <class event_type>
    class tmpl_async_event_queue : public async_event_queue_interface
    {
    public:
        virtual ~tmpl_async_event_queue() = default;

        explicit tmpl_async_event_queue() {}

        void push(event_type&& event)
        {
            std::lock_guard lock(mutex_);
            pending_events_.push_back(std::move(event));
        }

        virtual void sync() override
        {
            std::lock_guard lock(mutex_);
            events_.swap(pending_events_);
            pending_events_.clear();
            pending_events_.reserve(events_.capacity());
        }

        virtual void emit(event_manager& evt_manager) override
        {
            evt_manager.emit(events_);
        }

    private:
        std::vector<event_type> events_;
        std::vector<event_type> pending_events_;
        std::mutex mutex_;
    };

public:
    explicit async_event_queue();

    inline std::size_t number_of_event_types() { return event_queues_.size(); }
    void reserve(std::size_t number_of_event_types);

    template <class event_type>
    inline void push(event_type&& event)
    {
        get_or_create_event_queue_<event_type>().push(std::move(event));
    }

    void sync();
    void emit_events(event_manager& evt_manager);
    void sync_and_emit_events(event_manager& evt_manager);

private:
    template <class event_type>
    inline tmpl_async_event_queue<event_type>& get_or_create_event_queue_()
    {
        std::size_t event_type_index = event_info::type_index<event_type>();

        async_event_queue_interface* event_queue_ptr = nullptr;
        {
            std::shared_lock lock(mutex_);
            if (event_type_index < event_queues_.size()) [[likely]]
                event_queue_ptr = event_queues_[event_type_index].get();
        }

        if (!event_queue_ptr) [[unlikely]]
        {
            std::unique_lock lock(mutex_);
            std::size_t min_required_size = event_type_index + 1;
            if (min_required_size > event_queues_.size())
                event_queues_.resize(min_required_size);
            std::unique_ptr event_queue_uptr = std::make_unique<tmpl_async_event_queue<event_type>>();
            event_queue_ptr = event_queue_uptr.get();
            event_queues_[event_type_index] = std::move(event_queue_uptr);
        }

        return *static_cast<tmpl_async_event_queue<event_type>*>(event_queue_ptr);
    }

private:
    std::vector<async_event_queue_interface_uptr> event_queues_;
    mutable std::shared_mutex mutex_;
};

}
}
