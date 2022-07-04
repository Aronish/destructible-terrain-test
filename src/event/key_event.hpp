#pragma once

#include "event.hpp"

namespace eng
{
    class KeyEvent : public Event
    {
    public:
        KeyEvent(EventType event_type, int key_code, Window & window) : Event(event_type, window), m_key_code(key_code) {}

        int const m_key_code;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(int key_code, Window & window) : KeyEvent(EventType::KEY_PRESSED, key_code, window) {}

        static EventType getStaticEventType()
        {
            return EventType::KEY_PRESSED;
        }
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(int key_code, Window & window) : KeyEvent(EventType::KEY_RELEASED, key_code, window) {}
        
        static EventType getStaticEventType()
        {
            return EventType::KEY_RELEASED;
        }
    };
}