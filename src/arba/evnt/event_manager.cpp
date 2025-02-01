#include <arba/evnt/event_manager.hpp>
#include <arba/evnt/event_box.hpp>

#include <algorithm>

inline namespace arba
{
namespace evnt
{
event_manager::event_manager()
{
}

event_manager::~event_manager()
{
    std::unique_lock lock(mutex_);
    for (event_box* dispatcher : event_boxes_)
    {
        assert(dispatcher);
        dispatcher->reset_listened_event_manager_();
    }
    event_boxes_.clear();
    event_signals_.clear();
}

std::size_t event_manager::number_of_event_types() const
{
    std::shared_lock lock(mutex_);
    return event_signals_.size();
}

void event_manager::connect(event_box& listening_event_box)
{
    listening_event_box.set_listened_event_manager_(*this);
    std::unique_lock lock(mutex_);
    event_boxes_.push_back(&listening_event_box);
}

void event_manager::disconnect(event_box& listening_event_box)
{
    std::unique_lock lock(mutex_);
    auto iter = std::find(event_boxes_.begin(), event_boxes_.end(), &listening_event_box);
    if (iter != event_boxes_.end()) [[likely]]
    {
        listening_event_box.reset_listened_event_manager_();
        std::iter_swap(iter, std::prev(event_boxes_.end()));
        event_boxes_.pop_back();
    }
}

void event_manager::reserve(std::size_t number_of_event_types)
{
    std::unique_lock lock(mutex_);
    event_signals_.reserve(number_of_event_types);
}

void event_manager::emit(event_box& event_box, bool pre_sync)
{
    if (pre_sync)
        event_box.event_queue_().sync_and_forward_events_to(*this);
    else
        event_box.event_queue_().forward_events_to(*this);
}

}
}
