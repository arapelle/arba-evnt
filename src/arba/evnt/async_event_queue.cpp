#include <arba/evnt/async_event_queue.hpp>

#include <ranges>
#include <span>

inline namespace arba
{
namespace evnt
{

std::size_t async_event_queue::number_of_event_types()
{
    std::shared_lock lock(mutex_);
    return event_queues_.size();
}

void async_event_queue::reserve(std::size_t number_of_event_types)
{
    std::unique_lock lock(mutex_);
    event_queues_.reserve(number_of_event_types);
}

void async_event_queue::sync()
{
    std::shared_lock lock(mutex_);
    for (auto& queue_uptr : event_queues_)
        if (queue_uptr)
            queue_uptr->sync();
}

void async_event_queue::sync_and_forward_events_to(event_manager& evt_manager)
{
    std::shared_lock lock(mutex_);
    for (auto& queue_uptr : event_queues_)
        if (queue_uptr)
        {
            queue_uptr->sync();
            queue_uptr->forward_events_to(evt_manager);
        }
}

void async_event_queue::forward_events_to(event_manager& evt_manager)
{
    std::shared_lock lock(mutex_);
    for (auto& queue_uptr : event_queues_)
        if (queue_uptr)
            queue_uptr->forward_events_to(evt_manager);
}

} // namespace evnt
} // namespace arba
