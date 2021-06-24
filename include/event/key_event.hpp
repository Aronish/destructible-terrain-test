#pragma once

#include "event.hpp"

namespace eng
{
    class KeyEvent : public Event
    {
    public:
        KeyEvent(EventType event_type, int key_code) : Event(event_type), m_key_code(key_code) {}

        int const m_key_code;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        explicit KeyPressedEvent(int key_code) : KeyEvent(EventType::KEY_PRESSED, key_code) {}

        static EventType getStaticEventType()
        {
            return EventType::KEY_PRESSED;
        }
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        explicit KeyReleasedEvent(int key_code) : KeyEvent(EventType::KEY_RELEASED, key_code) {}
        
        static EventType getStaticEventType()
        {
            return EventType::KEY_RELEASED;
        }
    };
}