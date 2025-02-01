#pragma once

#include "event_manager.hpp"

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>

inline namespace arba
{
namespace evnt
{

class async_event_queue
{
private:
    class async_event_queue_interface_
    {
    public:
        virtual ~async_event_queue_interface_() = default;
        virtual void forward_events_to(event_manager& evt_manager) = 0;
        virtual void sync() = 0;
    };
    using async_event_queue_interface_uptr_ = std::unique_ptr<async_event_queue_interface_>;

    template <class event_type>
    class async_event_queue_ : public async_event_queue_interface_
    {
    public:
        virtual ~async_event_queue_() override
        {
            std::scoped_lock lock(pending_events_mutex_, events_mutex_);
            events_.clear();
            pending_events_.clear();
        }

        void push(event_type&& event)
        {
            std::lock_guard lock(pending_events_mutex_);
            pending_events_.push_back(std::move(event));
        }

        virtual void sync() override
        {
            std::scoped_lock lock(pending_events_mutex_, events_mutex_);
            events_.swap(pending_events_);
            pending_events_.clear();
            pending_events_.reserve(events_.capacity());
        }

        virtual void forward_events_to(event_manager& evt_manager) override
        {
            std::lock_guard lock(events_mutex_);
            evt_manager.emit(events_);
        }

    private:
        std::mutex events_mutex_;
        std::mutex pending_events_mutex_;
        std::vector<event_type> events_;
        std::vector<event_type> pending_events_;
    };

public:
    std::size_t number_of_event_types();
    void reserve(std::size_t number_of_event_types);

    template <class event_type>
    inline void push(event_type&& event)
    {
        get_or_create_event_queue_<event_type>().push(std::move(event));
    }

    void sync();
    void forward_events_to(event_manager& evt_manager);
    void sync_and_forward_events_to(event_manager& evt_manager);

private:
    template <class event_type>
    inline async_event_queue_<event_type>& get_or_create_event_queue_()
    {
        std::size_t event_type_index = event_info::type_index<event_type>();

        async_event_queue_interface_* event_queue_ptr = nullptr;
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
            std::unique_ptr event_queue_uptr = std::make_unique<async_event_queue_<event_type>>();
            event_queue_ptr = event_queue_uptr.get();
            event_queues_[event_type_index] = std::move(event_queue_uptr);
        }

        return *static_cast<async_event_queue_<event_type>*>(event_queue_ptr);
    }

private:
    mutable std::shared_mutex mutex_;
    std::vector<async_event_queue_interface_uptr_> event_queues_;
};

} // namespace evnt
} // namespace arba
