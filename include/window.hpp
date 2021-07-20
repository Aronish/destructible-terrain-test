#pragma once

#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "event/event.hpp"
#include "logger.hpp"

namespace eng
{
    class Window
    {
    private:
        using EventCallback = std::function<void(Event const &)>;

        GLFWwindow * m_window_handle;
        EventCallback m_event_callback;
        bool m_cursor_visible;

    public:
        Window(unsigned int width, unsigned int height, char const * title, EventCallback event_callback);

        ~Window();

        void setTitle(char const * title) const;

        void setCursorVisibility(bool visible);

        bool isCursorVisible() const;

        GLFWwindow * getWindowHandle() const;
    };
}