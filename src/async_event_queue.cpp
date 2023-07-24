#include <arba/evnt/async_event_queue.hpp>

inline namespace arba
{
namespace evnt
{

async_event_queue::async_event_queue_interface::~async_event_queue_interface()
{
}

void async_event_queue::sync()
{
    for (async_event_queue_interface_uptr& event_queue : event_queues_)
        if (event_queue)
            event_queue->sync();
}

void async_event_queue::sync_and_emit_events(event_manager& evt_manager)
{
    for (async_event_queue_interface_uptr& event_queue : event_queues_)
        if (event_queue)
        {
            event_queue->sync();
            event_queue->emit(evt_manager);
        }
}

void async_event_queue::emit_events(event_manager& evt_manager)
{
    for (const async_event_queue_interface_uptr& event_queue : event_queues_)
        if (event_queue)
            event_queue->emit(evt_manager);
}

}
}
