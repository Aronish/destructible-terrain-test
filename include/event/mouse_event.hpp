#pragma once

#include "event.hpp"

namespace eng
{
    class MouseButtonEvent : public Event
    {
    public:
        MouseButtonEvent(EventType event_type, int button_code, Window & window) : Event(event_type, window), m_button_code(button_code) {}

        int const m_button_code;
    };

    class MousePressedEvent : public MouseButtonEvent
    {
    public:
        explicit MousePressedEvent(int button_code, Window & window) : MouseButtonEvent(EventType::MOUSE_PRESSED, button_code, window) {}
        
        static EventType getStaticEventType()
        {
            return EventType::MOUSE_PRESSED;
        }
    };

    class MouseReleasedEvent : public MouseButtonEvent
    {
    public:
        explicit MouseReleasedEvent(int button_code, Window & window) : MouseButtonEvent(EventType::MOUSE_RELEASED, button_code, window) {}

        static EventType getStaticEventType()
        {
            return EventType::MOUSE_RELEASED;
        }
    };

    class MouseMovedEvent : public Event
    {
    public:
        MouseMovedEvent(double x_pos, double y_pos, Window & window) : Event(EventType::MOUSE_MOVED, window), m_x_pos(x_pos), m_y_pos(y_pos) {}

        double const m_x_pos, m_y_pos;
        
        static EventType getStaticEventType()
        {
            return EventType::MOUSE_MOVED;
        }
    };

    class MouseScrolledEvent : public Event
    {
    public:
        MouseScrolledEvent(double x_offset, double y_offset, Window & window) : Event(EventType::MOUSE_SCROLLED, window), m_x_offset(x_offset), m_y_offset(y_offset) {}

        double const m_x_offset, m_y_offset;
    
        static EventType getStaticEventType()
        {
            return EventType::MOUSE_SCROLLED;
        }
    };
}