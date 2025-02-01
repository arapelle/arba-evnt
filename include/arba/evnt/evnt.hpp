#pragma once

#include "async_event_queue.hpp"
#include "event_box.hpp"
#include "event_listener.hpp"
#include "event_manager.hpp"

inline namespace arba
{
namespace evnt
{

template <class event_type>
inline void event_listener_base::break_connection(std::size_t connection)
{
    evnt::event_manager* evt_manager = event_manager_;
    if (evt_manager)
    {
        this->invalidate();
        evt_manager->template disconnect<event_type>(connection);
    }
}

template <class event_type>
void event_manager::emit_to_event_boxes_(event_type& event)
{
    std::lock_guard lock(mutex_);
    for (event_box* evt_box : event_boxes_)
    {
        assert(evt_box);
        evt_box->receive_<event_type>(event);
    }
}

} // namespace evnt
} // namespace arba
