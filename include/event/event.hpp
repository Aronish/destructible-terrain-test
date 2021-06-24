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

    class Event
    {
    public:
        explicit Event(EventType event_type) : m_event_type(event_type) {}

        EventType const m_event_type;
    };

    namespace EventDispatcher
    {
        template<typename E>
        static void dispatch(Event const & event, void(*event_callback)(E const &))
        {
            if (event.m_event_type == E::getStaticEventType())
            {
                event_callback(static_cast<E const &>(event));
            }
        }

        template<typename E, typename T>
        static void dispatch(Event const & event, void(T::*event_callback)(E const &), T * instance)
        {
            if (event.m_event_type == E::getStaticEventType())
            {
                (instance->*event_callback)(static_cast<E const &>(event));
            }
        }
    };
}