#pragma once

#include "event_listener.hpp"
#include "event_info.hpp"
#include "signal.hpp"
#include <memory>
#include <atomic>
#include <functional>
#include <type_traits>
#include <utility>
#include <mutex>
#include <cassert>

inline namespace arba
{
namespace evnt
{

class event_box;

class event_manager
{
private:
    class event_signal_interface
    {
    public:
        virtual ~event_signal_interface() {}
    };
    using event_signal_interface_uptr = std::unique_ptr<event_signal_interface>;

    template <class event_type>
    class event_signal : public event_signal_interface
    {
        using evt_signal = signal<void(event_type&)>;

    public:
        using listener_function = typename evt_signal::CbFunction;

        explicit event_signal(std::mutex& mutex) : mutex_(&mutex) {}

        virtual ~event_signal() = default;

        template <class evt_listener>
        void connect(evt_listener& listener)
        {
            std::lock_guard lock(*mutex_);
            void(evt_listener::*receive)(event_type&) = &evt_listener::receive;
            listener_function function = std::bind(receive, &listener, std::placeholders::_1);
            std::size_t connection = signal_.connect(std::move(function));
            listener.as_listener(static_cast<const event_type*>(nullptr))->set_connection(connection);
        }

        inline void connect(listener_function&& listener)
        {
            std::lock_guard lock(*mutex_);
            signal_.connect(listener);
        }

        inline void disconnect(std::size_t connection)
        {
            std::lock_guard lock(*mutex_);
            signal_.disconnect(connection);
        }

        inline void emit(event_type& event)
        {
            std::lock_guard lock(*mutex_);
            signal_.emit(event);
        }

    private:
        std::mutex* mutex_;
        evt_signal signal_;
    };

public:
    template <class event_type>
    using receiver_function = typename event_signal<event_type>::listener_function;

    explicit event_manager(std::size_t max_number_event_types = 0);
    ~event_manager();
    event_manager(const event_manager&) = delete;
    event_manager& operator=(const event_manager&) = delete;

    inline std::size_t max_number_of_event_types() { return max_number_event_types_; }
    void resize(std::size_t number_of_event_types);

    // Connect:

    template <class event_type, class receiver_type>
    inline void connect(receiver_type& listener)
    {
        get_or_create_event_signal_<event_type>().connect(listener);
        listener.set_event_manager(*this);
    }

    template <class event_type>
    inline void connect(receiver_function<event_type> listener)
    {
        get_or_create_event_signal_<event_type>().connect(std::move(listener));
    }

    void connect(event_box& dispatcher);

    template <class event_type>
    inline void disconnect(std::size_t connection)
    {
        event_signal_ptr_<event_type>()->disconnect(connection);
    }

    // Disconnect:

    void disconnect(event_box& dispatcher);

    // Emit events:

    template <class event_type>
    inline void emit(event_type& event)
    {
        event_signal<event_type>* event_signal_ptr = event_signal_ptr_<event_type>();
        if (event_signal_ptr)
            event_signal_ptr->emit(event);
        emit_to_event_boxes_(event);
    }

    template <class event_type>
    inline void emit(event_type&& event)
    {
        event_type evt = std::move(event);
        emit<event_type>(std::ref(evt));
    }

    template <class event_type>
    inline void emit(std::vector<event_type>& events)
    {
        event_signal<event_type>* event_signal_ptr = event_signal_ptr_<event_type>();
        if (event_signal_ptr)
            for (event_type& event : events)
                event_signal_ptr->emit(event);
        for (event_type& event : events)
            emit_to_event_boxes_(event);
    }

    void emit(event_box& event_box, bool pre_sync = true);

private:
    struct signal_uptr_mt
    {
        event_signal_interface_uptr signal_uptr;
        std::mutex mutex;
    };

    template <class event_type>
    inline std::size_t event_type_index_()
    {
        std::size_t index = event_info::type_index<event_type>();
        if (index >= max_number_event_types_) [[unlikely]]
        {
            if (max_number_event_types_ == 0)
                throw std::runtime_error("Did you forget to call resize() ?");
            else
                throw std::runtime_error("Too many event types to handle !");
        }
        return index;
    }

    template <class event_type>
    inline event_signal<event_type>* event_signal_ptr_()
    {
        std::size_t index = event_type_index_<event_type>();
        auto& nth_element = event_signals_[index];
        std::lock_guard lock(nth_element.mutex);
        return static_cast<event_signal<event_type>*>(nth_element.signal_uptr.get());
    }

    template <class event_type>
    inline event_signal<event_type>& get_or_create_event_signal_()
    {
        std::size_t index = event_type_index_<event_type>();
        auto& nth_element = event_signals_[index];
        std::lock_guard lock(nth_element.mutex);
        event_signal_interface_uptr& event_signal_uptr = nth_element.signal_uptr;
        if (!event_signal_uptr)
        {
            event_signal_interface_uptr n_event = std::make_unique<event_signal<event_type>>(nth_element.mutex);
            event_signal_uptr = std::move(n_event);
        }

        return *static_cast<event_signal<event_type>*>(event_signal_uptr.get());
    }

    template <class event_type>
    void emit_to_event_boxes_(event_type& event);

private:
    std::size_t max_number_event_types_;
    std::unique_ptr<signal_uptr_mt[]> event_signals_;
    std::vector<event_box*> event_boxs_;
    std::mutex mutex_;
};

}
}
