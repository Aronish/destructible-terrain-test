#pragma once

#include "event.hpp"

namespace eng
{
    class WindowResizedEvent : public Event
    {
    public:
        WindowResizedEvent(int width, int height) : Event(EventType::WINDOW_RESIZED), m_width(width), m_height(height) {}

        int const m_width, m_height;

        static EventType getStaticEventType()
        {
            return EventType::WINDOW_RESIZED;
        }
    };
}