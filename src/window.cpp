#include "event/key_event.hpp"
#include "event/mouse_event.hpp"
#include "event/application_event.hpp"

#include "window.hpp"

namespace eng
{
    Window::Window(int unsigned width, int unsigned height, char const * title, bool maximized, EventCallback event_callback)
        : m_width(width), m_height(height), m_init_width(width), m_init_height(height), m_maximized(maximized), m_user_pointer(std::move(event_callback), *this)
    {
        if (!glfwInit()) ENG_LOG("[GLFW]: glfwInit failed!");
#ifdef ENG_DEBUG
        glfwSetErrorCallback([](int error_code, char const * description)
        {
            ENG_LOG_F("(%d): %s", error_code, description);
        });
        glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
#endif
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, maximized ? GLFW_TRUE : GLFW_FALSE);

        m_video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        m_window_handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!m_window_handle) ENG_LOG("[GLFW]: Window could not be created!");

        if (!maximized) glfwSetWindowPos(m_window_handle, (m_video_mode->width - width) / 2, (m_video_mode->height - height) / 2);
        
        glfwMakeContextCurrent(m_window_handle);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

#define ENG_GL_DEBUG_LOG 1
#if defined(ENG_DEBUG) && ENG_GL_DEBUG_LOG
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(
        [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const * message, void const *)
        {
            if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
            ENG_LOG_F("Source: %x, Type: %x, Id: %d, Severity: %x, Length: %d\nMessage: %s\n", source, type, id, severity, length, message);
        }, nullptr);
#endif

        glfwSetWindowUserPointer(m_window_handle, &m_user_pointer);

        setCursorVisibility(true);

        // Event Callbacks
        glfwSetKeyCallback(m_window_handle, [](GLFWwindow * window_handle, int key, int, int action, int)
        {
            auto const & user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
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
            auto const & user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
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
            auto const & user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
            user_pointer.m_event_callback(MouseScrolledEvent(x_offset, y_offset, user_pointer.m_window));
        });

        glfwSetCursorPosCallback(m_window_handle, [](GLFWwindow * window_handle, double x_pos, double y_pos)
        {
            auto const & user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
            user_pointer.m_event_callback(MouseMovedEvent(x_pos, y_pos, user_pointer.m_window));
            user_pointer.m_window.m_mouse_x = x_pos;
            user_pointer.m_window.m_mouse_y = y_pos;
        });

        glfwSetFramebufferSizeCallback(m_window_handle, [](GLFWwindow * window_handle, int width, int height)
        {
            auto const & user_pointer = *static_cast<UserPointer*>(glfwGetWindowUserPointer(window_handle));
            user_pointer.m_window.setSize(width, height);
            user_pointer.m_event_callback(WindowResizedEvent(width, height, user_pointer.m_window));
        });

        glfwSetWindowMaximizeCallback(m_window_handle, [](GLFWwindow * window_handle, int state)
        {
            auto const & user_pointer = *static_cast<UserPointer *>(glfwGetWindowUserPointer(window_handle));
            user_pointer.m_window.setMaximized(state);
        });
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_window_handle);
        glfwTerminate();
    }

    void Window::setSize(int unsigned width, int unsigned height)
    {
        m_width = width;
        m_height = height;
        glViewport(0, 0, width, height);
    }

    int Window::getWidth() const
    {
        return m_width;
    }

    int Window::getHeight() const
    {
        return m_height;
    }

    void Window::setMaximized(bool maximized)
    {
        m_maximized = maximized;
    }

    bool Window::isMaximized() const
    {
        return m_maximized;
    }

    void Window::setFullscreen(bool fullscreen)
    {
        m_fullscreen = fullscreen;
        if (fullscreen)
        {
            glfwSetWindowMonitor(m_window_handle, glfwGetPrimaryMonitor(), 0, 0, m_video_mode->width, m_video_mode->height, 60);
        }
        else
        {
            if (m_maximized)
            {
                glfwSetWindowMonitor(m_window_handle, 0, 0, 0, m_video_mode->width, m_video_mode->height - 61, 60);
                glfwMaximizeWindow(m_window_handle);
            } else glfwSetWindowMonitor(m_window_handle, 0, m_video_mode->width / 2 - m_init_width / 2, m_video_mode->height / 2 - m_init_height / 2, m_init_width, m_init_height, 60);
        }
    }

    bool Window::isFullscreen() const
    {
        return m_fullscreen;
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