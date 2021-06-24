#pragma once

#include <memory>

#include "event/event.hpp"
#include "graphics/vertex_buffer.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/shader.hpp"

#include <GLFW/glfw3.h>

namespace eng
{
    class Application
    {
    private:
        std::shared_ptr<VertexArray> m_vao;
        std::shared_ptr<VertexBuffer> m_vbo;
        std::shared_ptr<Shader> m_shader;

    public:
        Application(int width, int height, char const * title);
        ~Application();

        void run();
        void onEvent(Event const & event);
        void update(float delta_time);
        void render();

    private:
        class Window
        {
        private:
            GLFWwindow * m_window_handle;

            struct UserPointer
            {
                Application * m_application;
                decltype(&Application::onEvent) m_event_callback = &Application::onEvent;

                UserPointer(Application * application) : m_application(application) {}

                void dispatchEvent(Event const& event)
                {
                    (m_application->*m_event_callback)(event);
                }
            } m_user_pointer;
        public:
            Window(int width, int height, char const * title, Application * application);
            ~Window();

            void setTitle(char const * title);

            GLFWwindow * getWindowHandle() const
            {
                return m_window_handle;
            }
        } m_window;
    };
}