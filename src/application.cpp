#include <stdio.h>
#include <functional>

#include <glad/glad.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

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

        m_camera.setPosition(glm::vec3{5.0f});

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO & io = ImGui::GetIO();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_window.getWindowHandle(), true);
        ImGui_ImplOpenGL3_Init("#version 460 core");
    }

    void Application::onEvent(Event const & event)
    {
        EventDispatcher::dispatch<WindowResizedEvent>(event, &FirstPersonCamera::onWindowResized, &m_camera);
        if (!event.m_window.isCursorVisible())
        {
            EventDispatcher::dispatch<MouseMovedEvent>(event, &FirstPersonCamera::onMouseMoved, &m_camera);
        }
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
        m_world.render(m_camera);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool values_changed = false;

        if (ImGui::InputInt("Grid Size", &m_grid_size, 1, 5))
        {
            if (m_grid_size < 1.0f) m_grid_size = 1.0f;
            values_changed = true;
        }
        values_changed |= ImGui::DragFloat("Surface Level", &m_surface_level, 0.01f, 0.0f, 1.0f);

        if (values_changed) m_world.generateWorld(m_grid_size, m_step_size, m_surface_level);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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