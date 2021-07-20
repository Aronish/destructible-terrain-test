#include "event/key_event.hpp"
#include "event/mouse_event.hpp"
#include "event/application_event.hpp"

#include "window.hpp"

namespace eng
{
    Window::Window(unsigned int width, unsigned int height, char const * title, EventCallback event_callback)
        : m_event_callback(event_callback)
    {
        if (!glfwInit()) ENG_LOG("[GLFW]: glfwInit failed!");
#ifdef ENG_DEBUG
        glfwSetErrorCallback([](int, char const * description)
        {
            ENG_LOG(description);
        });
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        m_window_handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!m_window_handle) ENG_LOG("[GLFW]: Window could not be created!");
        
        glfwMakeContextCurrent(m_window_handle);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        glfwSetWindowUserPointer(m_window_handle, &m_event_callback);

        setCursorVisibility(true);

        // Event Callbacks
        glfwSetKeyCallback(m_window_handle, [](GLFWwindow * window_handle, int key, int, int action, int)
        {
            auto event_callback = *static_cast<EventCallback*>(glfwGetWindowUserPointer(window_handle));
            switch (action)
            {
                case GLFW_PRESS:
                {
                    event_callback(KeyPressedEvent(key));
                    break;
                }
                case GLFW_RELEASE:
                {
                    event_callback(KeyReleasedEvent(key));
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_window_handle, [](GLFWwindow * window_handle, int button, int action, int)
        {
            auto event_callback = *static_cast<EventCallback*>(glfwGetWindowUserPointer(window_handle));
            switch (action)
            {
                case GLFW_PRESS:
                {
                    event_callback(MousePressedEvent(button));
                    break;
                }
                case GLFW_RELEASE:
                {
                    event_callback(MouseReleasedEvent(button));
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_window_handle, [](GLFWwindow * window_handle, double x_offset, double y_offset)
        {
            auto event_callback = *static_cast<EventCallback*>(glfwGetWindowUserPointer(window_handle));
            event_callback(MouseScrolledEvent(x_offset, y_offset));
        });

        glfwSetCursorPosCallback(m_window_handle, [](GLFWwindow * window_handle, double x_pos, double y_pos)
        {
            auto event_callback = *static_cast<EventCallback*>(glfwGetWindowUserPointer(window_handle));
            event_callback(MouseMovedEvent(x_pos, y_pos));
        });

        glfwSetWindowSizeCallback(m_window_handle, [](GLFWwindow * window_handle, int width, int height)
        {
            auto event_callback = *static_cast<EventCallback*>(glfwGetWindowUserPointer(window_handle));
            event_callback(WindowResizedEvent(width, height));
        });
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_window_handle);
        glfwTerminate();
    }

    void Window::setTitle(char const * title) const
    {
        glfwSetWindowTitle(m_window_handle, title);
    }

    void Window::setCursorVisibility(bool visible)
    {
        m_cursor_visible = visible;
        if (visible)
        {
            glfwSetInputMode(m_window_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else
        {
            glfwSetInputMode(m_window_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    bool Window::isCursorVisible() const
    {
        return m_cursor_visible;
    }

    GLFWwindow * Window::getWindowHandle() const
    {
        return m_window_handle;
    }
}