#pragma once

namespace eng
{
    enum class EventType
    {
        KEY_PRESSED, KEY_RELEASED,
        MOUSE_PRESSED, MOUSE_RELEASED,
        MOUSE_SCROLLED,
        MOUSE_MOVED,
        WINDOW_RESIZED
    };

    class Window;

    class Event
    {
    public:
        Event(EventType event_type, Window & window) : m_event_type(event_type), m_window(window) {}

        EventType const m_event_type;
        Window & m_window;
    };

    template<typename E>
    concept has_static_event_type = requires
    {
        E::getStaticEventType();
    };

    namespace EventDispatcher
    {
        template<has_static_event_type E>
        static void dispatch(Event const & event, void(*event_callback)(E const &))
        {
            if (event.m_event_type == E::getStaticEventType())
            {
                event_callback(static_cast<E const &>(event));
            }
        }

        template<has_static_event_type E, typename T>
        static void dispatch(Event const & event, void(T::*event_callback)(E const &), T * instance)
        {
            if (event.m_event_type == E::getStaticEventType())
            {
                (instance->*event_callback)(static_cast<E const &>(event));
            }
        }
    };
}