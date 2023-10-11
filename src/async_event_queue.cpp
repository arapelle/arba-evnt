#include <arba/evnt/async_event_queue.hpp>
#include <ranges>
#include <span>

inline namespace arba
{
namespace evnt
{

async_event_queue::async_event_queue()
{
}

void async_event_queue::reserve(std::size_t number_of_event_types)
{
    event_queues_.reserve(number_of_event_types);
}

void async_event_queue::sync()
{
    for (auto& queue_uptr : event_queues_)
        if (queue_uptr)
            queue_uptr->sync();
}

void async_event_queue::sync_and_emit_events(event_manager& evt_manager)
{
    for (auto& queue_uptr : event_queues_)
        if (queue_uptr)
        {
            queue_uptr->sync();
            queue_uptr->emit(evt_manager);
        }
}

void async_event_queue::emit_events(event_manager& evt_manager)
{
    for (auto& queue_uptr : event_queues_)
        if (queue_uptr)
            queue_uptr->emit(evt_manager);
}

}
}
