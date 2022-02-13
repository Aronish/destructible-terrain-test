#pragma once

#include <functional>
#include <utility>
#include <vector>

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
        bool m_cursor_visible;
        double m_mouse_x, m_mouse_y;

        struct UserPointer
        {
            EventCallback m_event_callback;
            Window & m_window;

            UserPointer(EventCallback event_callback, Window & window) : m_event_callback(std::move(event_callback)), m_window(window) {}
        } m_user_pointer;

    public:
        Window(unsigned int width, unsigned int height, char const * title, EventCallback event_callback);

        ~Window();

        void setSize(unsigned int width, unsigned int height);

        void setTitle(char const * title) const;

        void setCursorVisibility(bool visible);

        bool isCursorVisible() const;

        std::pair<double, double> getCursorPosition() const;

        GLFWwindow * getWindowHandle() const;
    };
}