#pragma once

#include "event_manager.hpp"
#include <mutex>
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
        virtual ~tmpl_async_event_queue() {}

        explicit tmpl_async_event_queue(std::mutex& mutex) : mutex_(&mutex) {}

        void push(event_type&& event)
        {
            std::lock_guard<std::mutex> lock(*mutex_);
            pending_events_.push_back(std::move(event));
        }

        virtual void sync() override
        {
            std::lock_guard<std::mutex> lock(*mutex_);
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
        std::mutex* mutex_;
    };

public:
    explicit async_event_queue(std::size_t max_number_event_types = 0);

    inline std::size_t max_number_of_event_types() { return max_number_event_types_; }
    void resize(std::size_t number_of_event_types);

    template <class event_type>
    inline void push(event_type&& event)
    {
        get_or_create_event_queue_<event_type>().push(std::move(event));
    }

    void sync();
    void emit_events(event_manager& evt_manager);
    void sync_and_emit_events(event_manager& evt_manager);

private:
    struct queue_uptr_mt
    {
        async_event_queue_interface_uptr queue_uptr;
        std::mutex mutex;
    };

    template <class event_type>
    inline std::size_t event_type_index_()
    {
        std::size_t index = event_info::type_index<event_type>();
        if (index >= max_number_event_types_) [[unlikely]]
        {
            if (max_number_event_types_ == 0)
                throw std::runtime_error("Did you forget to call resize() ?");
            else
                throw std::runtime_error("Too many event types to handle !");
        }
        return index;
    }

    template <class event_type>
    inline tmpl_async_event_queue<event_type>& get_or_create_event_queue_()
    {
        std::size_t index = event_type_index_<event_type>();
        auto& nth_element = event_queues_[index];
        std::lock_guard lock(nth_element.mutex);
        async_event_queue_interface_uptr& async_event_queue_uptr = nth_element.queue_uptr;
        if (!async_event_queue_uptr)
        {
            async_event_queue_interface_uptr n_queue =
                std::make_unique<tmpl_async_event_queue<event_type>>(nth_element.mutex);
            async_event_queue_uptr = std::move(n_queue);
        }

        return *static_cast<tmpl_async_event_queue<event_type>*>(async_event_queue_uptr.get());
    }

private:
    std::size_t max_number_event_types_;
    std::unique_ptr<queue_uptr_mt[]> event_queues_;
};

}
}
