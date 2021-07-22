#include <stdio.h>
#include <functional>

#include <glad/glad.h>

#include "logger.hpp"
#include "event/key_event.hpp"
#include "event/mouse_event.hpp"
#include "event/application_event.hpp"

#include "application.hpp"

namespace eng
{
    Application::Application(unsigned int width, unsigned int height, char const * title)
        : m_window(width, height, title, std::bind(&Application::onEvent, this, std::placeholders::_1)), m_camera(width, height)
    {
        glfwSwapInterval(1);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.3f, 0.8f, 0.2f, 1.0f);

        int indices[] =
        {
            0, 1, 2,
            0, 2, 3
        };
        m_vao = std::make_shared<VertexArray>(indices, sizeof(indices));

        float vertices[] =
        {
            0.0f, 0.0f, 0.0f, 0.0f,
            0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 1.0f, 1.0f,
            0.0f, 0.5f, 0.0f, 1.0f
        };

        m_vbo = std::make_shared<VertexBuffer>(vertices, sizeof(vertices), VertexBufferLayout{{{2, GL_FLOAT}, {2, GL_FLOAT}}});

        m_vao->setVertexBuffer(m_vbo);

        m_shader = std::make_shared<Shader>("res/shaders/shader.glsl");

        m_texture = std::make_shared<Texture>("res/textures/the_one_ring.png");
    }

    void Application::onEvent(Event const & event)
    {
        EventDispatcher::dispatch<WindowResizedEvent>(event, &FirstPersonCamera::onWindowResized, &m_camera);
        if (!event.m_window.isCursorVisible()) EventDispatcher::dispatch<MouseMovedEvent>(event, &FirstPersonCamera::onMouseMoved, &m_camera);
        EventDispatcher::dispatch<KeyPressedEvent>(event, [](KeyPressedEvent const & event)
        {
            if (event.m_key_code == GLFW_KEY_E)
            {
                event.m_window.setCursorVisibility(!event.m_window.isCursorVisible());
            }
        });
    }

    void Application::update(float delta_time)
    {
        glfwPollEvents();
        m_camera.update(delta_time, m_window);
    }

    void Application::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_shader->bind();
        m_shader->setUniformMatrix4f("u_model", glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
        m_shader->setUniformMatrix4f("u_view", m_camera.getViewMatrix());
        m_shader->setUniformMatrix4f("u_projection", m_camera.getProjectionMatrix());
        m_vao->bind();
        m_texture->bind(0);
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
}