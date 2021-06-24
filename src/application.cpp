#include <stdio.h>

#include <glad/glad.h>

#include "logger.hpp"
#include "event/key_event.hpp"
#include "event/mouse_event.hpp"
#include "event/application_event.hpp"

#include "application.hpp"

namespace eng
{
    Application::Application(int width, int height, char const * title)
        : m_window(width, height, title, this)
    {
        glfwSwapInterval(1);
        glDisable(GL_DEPTH_TEST);

        int indices[] =
        {
            0, 1, 2,
            0, 2, 3
        };
        m_vao = std::make_shared<VertexArray>(indices, sizeof(indices));

        float vertices[] =
        {
            0.0f, 0.0f,
            0.5f, 0.0f,
            0.5f, 0.5f,
            0.0f, 0.5f
        };

        m_vbo = std::make_shared<VertexBuffer>(vertices, sizeof(vertices), VertexBufferLayout{{{2, GL_FLOAT}}});

        m_vao->bind();
        m_vbo->bind();
        m_vao->setVertexBuffer(m_vbo);

        m_shader = std::make_shared<Shader>("res/shaders/shader.glsl");
    }

    Application::~Application()
    {
    }

    void Application::onEvent(Event const & event)
    {
    }

    void Application::update(float delta_time)
    {
        glfwPollEvents();
    }

    void Application::render()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        m_shader->bind();
        m_vao->bind();
        m_vao->drawElements();
        glfwSwapBuffers(m_window.getWindowHandle());
    }

    void Application::run()
    {
        int frames = 0, updates = 0;
        double const dt = 1.0 / 60.0;
        double time = 0.0;
        double current_time = glfwGetTime();

        while (!glfwWindowShouldClose(m_window.getWindowHandle()))
        {
            double new_time = glfwGetTime();
            double frame_time = new_time - current_time;
            current_time = new_time;

            if (time > 1.0)
            {
                char frame_data[25];
                sprintf_s(frame_data, sizeof(frame_data), "FPS: %d, UPS: %d", frames, updates);
                m_window.setTitle(frame_data);
                frames = 0;
                updates = 0;
                time = 0.0;
            }

            while (frame_time > 0.0)
            {
                double delta_time = frame_time < dt ? frame_time : dt;
                update((float)delta_time);
                frame_time -= delta_time;
                time += delta_time;
                ++updates;
            }
            render();
            ++frames;
        }
    }
    
    Application::Window::Window(int width, int height, char const * title, Application * application)
        : m_user_pointer(application)
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

        // Event Callbacks
        glfwSetKeyCallback(m_window_handle, [](GLFWwindow * window_handle, int key, int, int action, int)
        {
            auto * window_user_pointer = (UserPointer *) glfwGetWindowUserPointer(window_handle);
            switch (action)
            {
                case GLFW_PRESS:
                {
                    window_user_pointer->dispatchEvent(KeyPressedEvent(key));
                    break;
                }
                case GLFW_RELEASE:
                {
                    window_user_pointer->dispatchEvent(KeyReleasedEvent(key));
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_window_handle, [](GLFWwindow * window_handle, int button, int action, int)
        {
            auto * window_user_pointer = (UserPointer *) glfwGetWindowUserPointer(window_handle);
            switch (action)
            {
                case GLFW_PRESS:
                {
                    window_user_pointer->dispatchEvent(MousePressedEvent(button));
                    break;
                }
                case GLFW_RELEASE:
                {
                    window_user_pointer->dispatchEvent(MouseReleasedEvent(button));
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_window_handle, [](GLFWwindow * window_handle, double x_offset, double y_offset)
        {
            auto * window_user_pointer = (UserPointer *) glfwGetWindowUserPointer(window_handle);
            window_user_pointer->dispatchEvent(MouseScrolledEvent(x_offset, y_offset));
        });

        glfwSetCursorPosCallback(m_window_handle, [](GLFWwindow * window_handle, double x_pos, double y_pos)
        {
            auto * window_user_pointer = (UserPointer *) glfwGetWindowUserPointer(window_handle);
            window_user_pointer->dispatchEvent(MouseMovedEvent(x_pos, y_pos));
        });

        glfwSetWindowSizeCallback(m_window_handle, [](GLFWwindow * window_handle, int width, int height)
        {
            auto * window_user_pointer = (UserPointer *) glfwGetWindowUserPointer(window_handle);
            window_user_pointer->dispatchEvent(WindowResizedEvent(width, height));
        });
    }

    Application::Window::~Window()
    {
        glfwDestroyWindow(m_window_handle);
        glfwTerminate();
    }

    void Application::Window::setTitle(char const * title)
    {
        glfwSetWindowTitle(m_window_handle, title);
    }
}