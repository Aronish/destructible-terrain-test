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
        GLFWvidmode const * m_video_mode;
        bool m_cursor_visible, m_fullscreen;
        double m_mouse_x, m_mouse_y;
        int m_width, m_height, m_init_width, m_init_height;

        struct UserPointer
        {
            EventCallback m_event_callback;
            Window & m_window;

            UserPointer(EventCallback event_callback, Window & window) : m_event_callback(std::move(event_callback)), m_window(window) {}
        } m_user_pointer;

    public:
        Window(int unsigned width, int unsigned height, char const * title, bool maximized, EventCallback event_callback);

        ~Window();

        void setSize(int unsigned width, int unsigned height);

        int getWidth() const;
        int getHeight() const;

        void setFullscreen(bool fullscreen);
        bool isFullscreen() const;

        void setTitle(char const * title) const;

        void setCursorVisibility(bool visible);

        bool isCursorVisible() const;

        std::pair<double, double> getCursorPosition() const;

        GLFWwindow * getWindowHandle() const;
    };
}