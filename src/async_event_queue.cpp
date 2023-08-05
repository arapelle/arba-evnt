#include <arba/evnt/async_event_queue.hpp>
#include <ranges>
#include <span>

inline namespace arba
{
namespace evnt
{

async_event_queue::async_event_queue(std::size_t max_number_event_types)
    : max_number_event_types_(max_number_event_types)
{
    if (max_number_event_types)
        resize(max_number_event_types);
}

void async_event_queue::resize(std::size_t number_of_event_types)
{
    if (max_number_event_types_ > 0) [[unlikely]]
        throw std::runtime_error("Resize can only be done once just at construction or just after.");
    event_queues_ = std::make_unique<queue_uptr_mt[]>(number_of_event_types);
    max_number_event_types_ = number_of_event_types;
}

void async_event_queue::sync()
{
    for (queue_uptr_mt& element : std::span(event_queues_.get(), max_number_event_types_))
        if (element.queue_uptr)
            element.queue_uptr->sync();
}

void async_event_queue::sync_and_emit_events(event_manager& evt_manager)
{
    for (queue_uptr_mt& element : std::span(event_queues_.get(), max_number_event_types_))
        if (element.queue_uptr)
        {
            element.queue_uptr->sync();
            element.queue_uptr->emit(evt_manager);
        }
}

void async_event_queue::emit_events(event_manager& evt_manager)
{
    for (const queue_uptr_mt& element : std::span(event_queues_.get(), max_number_event_types_))
        if (element.queue_uptr)
            element.queue_uptr->emit(evt_manager);
}

}
}
