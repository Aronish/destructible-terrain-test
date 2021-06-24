#pragma once

#include "event.hpp"

namespace eng
{
    class MouseButtonEvent : public Event
    {
    public:
        MouseButtonEvent(EventType event_type, int button_code) : Event(event_type), m_button_code(button_code) {}

        int const m_button_code;
    };

    class MousePressedEvent : public MouseButtonEvent
    {
    public:
        explicit MousePressedEvent(int button_code) : MouseButtonEvent(EventType::MOUSE_PRESSED, button_code) {}
        
        static EventType getStaticEventType()
        {
            return EventType::MOUSE_PRESSED;
        }
    };

    class MouseReleasedEvent : public MouseButtonEvent
    {
    public:
        explicit MouseReleasedEvent(int button_code) : MouseButtonEvent(EventType::MOUSE_RELEASED, button_code) {}

        static EventType getStaticEventType()
        {
            return EventType::MOUSE_RELEASED;
        }
    };

    class MouseMovedEvent : public Event
    {
    public:
        MouseMovedEvent(double x_pos, double y_pos) : Event(EventType::MOUSE_MOVED), m_x_pos(x_pos), m_y_pos(y_pos) {}

        double const m_x_pos, m_y_pos;
        
        static EventType getStaticEventType()
        {
            return EventType::MOUSE_MOVED;
        }
    };

    class MouseScrolledEvent : public Event
    {
    public:
        MouseScrolledEvent(double x_offset, double y_offset) : Event(EventType::MOUSE_SCROLLED), m_x_offset(x_offset), m_y_offset(y_offset) {}

        double const m_x_offset, m_y_offset;
    
        static EventType getStaticEventType()
        {
            return EventType::MOUSE_SCROLLED;
        }
    };
}