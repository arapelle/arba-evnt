#pragma once

#include "async_event_queue.hpp"

inline namespace arba
{
namespace evnt
{

class event_manager;

class event_box
{
public:
    ~event_box();

    [[nodiscard]] inline const evnt::event_manager* listened_event_manager() const noexcept
    {
        return listened_event_manager_;
    }

private:
    friend class event_manager;

    void set_listened_event_manager_(evnt::event_manager& listened_event_manager);
    void reset_listened_event_manager_();

    template <class event_type>
    inline void receive_(event_type& event)
    {
        evt_queue_.push(event_type(event));
    }

    inline async_event_queue& event_queue_() { return evt_queue_; }

private:
    std::mutex mutex_;
    evnt::event_manager* listened_event_manager_ = nullptr;
    async_event_queue evt_queue_;
};

} // namespace evnt
} // namespace arba
