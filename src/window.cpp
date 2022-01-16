#include "event/key_event.hpp"
#include "event/mouse_event.hpp"
#include "event/application_event.hpp"

#include "window.hpp"

namespace eng
{
    Window::Window(unsigned int width, unsigned int height, char const * title, EventCallback event_callback)
        : m_user_pointer(std::move(event_callback), *this)
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

        glfwSetWindowUserPointer(m_window_handle, &m_user_pointer);

        setCursorVisibility(true);

        // Event Callbacks
        glfwSetKeyCallback(m_window_handle, [](GLFWwindow * window_handle, int key, int, int action, int)
        {
            auto user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
            switch (action)
            {
                case GLFW_PRESS:
                {
                    user_pointer.m_event_callback(KeyPressedEvent(key, user_pointer.m_window));
                    break;
                }
                case GLFW_RELEASE:
                {
                    user_pointer.m_event_callback(KeyReleasedEvent(key, user_pointer.m_window));
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_window_handle, [](GLFWwindow * window_handle, int button, int action, int)
        {
            auto user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
            switch (action)
            {
                case GLFW_PRESS:
                {
                    user_pointer.m_event_callback(MousePressedEvent(button, user_pointer.m_window));
                    break;
                }
                case GLFW_RELEASE:
                {
                    user_pointer.m_event_callback(MouseReleasedEvent(button, user_pointer.m_window));
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_window_handle, [](GLFWwindow * window_handle, double x_offset, double y_offset)
        {
            auto user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
            user_pointer.m_event_callback(MouseScrolledEvent(x_offset, y_offset, user_pointer.m_window));
        });

        glfwSetCursorPosCallback(m_window_handle, [](GLFWwindow * window_handle, double x_pos, double y_pos)
        {
            auto user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
            user_pointer.m_event_callback(MouseMovedEvent(x_pos, y_pos, user_pointer.m_window));
            user_pointer.m_window.m_mouse_x = x_pos;
            user_pointer.m_window.m_mouse_y = y_pos;
        });

        glfwSetWindowSizeCallback(m_window_handle, [](GLFWwindow * window_handle, int width, int height)
        {
            auto user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
            user_pointer.m_window.setSize(width, height);
            user_pointer.m_event_callback(WindowResizedEvent(width, height, user_pointer.m_window));
        });
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_window_handle);
        glfwTerminate();
    }

    void Window::setSize(unsigned int width, unsigned int height)
    {
        glViewport(0, 0, width, height);
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

    std::pair<double, double> Window::getCursorPosition() const
    {
        return { m_mouse_x, m_mouse_y };
    }

    GLFWwindow * Window::getWindowHandle() const
    {
        return m_window_handle;
    }
}