#include <arba/evnt/event_box.hpp>

inline namespace arba
{
namespace evnt
{

event_box::~event_box()
{
    std::lock_guard lock(mutex_);
    if (listened_event_manager_)
    {
        listened_event_manager_->disconnect(*this);
        listened_event_manager_ = nullptr;
    }
}

void event_box::set_listened_event_manager_(evnt::event_manager& listened_event_manager)
{
    std::lock_guard lock(mutex_);
    if (listened_event_manager_) [[unlikely]]
        throw std::runtime_error("This event_box is already connected to an emitting event_manager.");
    listened_event_manager_ = &listened_event_manager;
    evt_queue_.reserve(listened_event_manager.number_of_event_types());
}

void event_box::reset_listened_event_manager_()
{
    if (mutex_.try_lock())
    {
        std::lock_guard lock(mutex_, std::adopt_lock);
        if (!listened_event_manager_) [[unlikely]]
            throw std::runtime_error("This event_box is expected to be connected to an emitting event_manager.");
        listened_event_manager_ = nullptr;
    }
}

} // namespace evnt
} // namespace arba
