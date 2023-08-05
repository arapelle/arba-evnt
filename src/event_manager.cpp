#include <arba/evnt/event_manager.hpp>
#include <arba/evnt/event_box.hpp>

inline namespace arba
{
namespace evnt
{
event_manager::event_manager(std::size_t max_number_event_types)
    : max_number_event_types_(0)
{
    if (max_number_event_types)
        resize(max_number_event_types);
}

event_manager::~event_manager()
{
    std::lock_guard lock(mutex_);
    for (event_box* dispatcher : event_boxs_)
    {
        assert(dispatcher);
        dispatcher->set_parent_event_manager(nullptr);
    }
}

void event_manager::connect(event_box& dispatcher)
{
    std::lock_guard<std::mutex> lock(mutex_);
    dispatcher.set_parent_event_manager(*this, max_number_event_types_);
    event_boxs_.push_back(&dispatcher);
}

void event_manager::disconnect(event_box& dispatcher)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = std::find(event_boxs_.begin(), event_boxs_.end(), &dispatcher);
    if (iter != event_boxs_.end())
    {
        dispatcher.set_parent_event_manager(nullptr);
        std::iter_swap(iter, std::prev(event_boxs_.end()));
        event_boxs_.pop_back();
    }
}

void event_manager::resize(std::size_t number_of_event_types)
{
    if (max_number_event_types_ > 0) [[unlikely]]
        throw std::runtime_error("Resize can only be done once just at construction or just after.");
    event_signals_ = std::make_unique<signal_uptr_mt[]>(number_of_event_types);
    max_number_event_types_ = number_of_event_types;
}

void event_manager::emit(event_box& event_box, bool pre_sync)
{
    if (pre_sync)
        event_box.event_queue().sync_and_emit_events(*this);
    else
        event_box.event_queue().emit_events(*this);
}

}
}
