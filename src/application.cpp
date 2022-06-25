#include <stdio.h>
#include <functional>
#include <cmath>

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
    Application::Application(unsigned int width, unsigned int height, char const * title, bool maximized)
        : m_window(width, height, title, maximized, std::bind(&Application::onEvent, this, std::placeholders::_1)), m_camera(width, height), m_world(m_game_system)
    {
        glfwSwapInterval(1);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.42f, 0.71f, 0.73f, 1.0f);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_window.getWindowHandle(), true);
        ImGui_ImplOpenGL3_Init("#version 460 core");

        m_crosshair_texture = m_game_system.getAssetManager().getTexture("res/textures/crosshair.png");
        m_textured_quad_shader = m_game_system.getAssetManager().getShader("res/shaders/textured_quad.glsl");

        int quad_indices[] = 
        {
            0, 2, 3, 0, 3, 1        //CW
        };

        float crosshair_width = static_cast<float>(m_crosshair_texture->getWidth());
        float crosshair_height = static_cast<float>(m_crosshair_texture->getHeight());
        float quad_vertices[] = 
        {
            0.0f,               0.0f,               0.0f, 0.0f,
            crosshair_width,    0.0f,               1.0f, 0.0f,
            0.0f,               crosshair_height,   0.0f, 1.0f,
            crosshair_width,    crosshair_height,   1.0f, 1.0f
        };

        m_crosshair_ib = m_game_system.getAssetManager().createBuffer();

        m_crosshair_vb = m_game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_crosshair_vb, sizeof(quad_vertices), quad_vertices, 0);

        m_crosshair_va = m_game_system.getAssetManager().createVertexArray();
        VertexArray::associateVertexBuffer(m_crosshair_va, m_crosshair_vb, VertexDataLayout::POSIITON_UV_2F);
        VertexArray::associateIndexBuffer(m_crosshair_va, m_crosshair_ib, quad_indices, sizeof(quad_indices));

        m_world.generateChunks();
    }

    void Application::onEvent(Event const & event)
    {
        EventDispatcher::dispatch<WindowResizedEvent>(event, &FirstPersonCamera::onWindowResized, &m_camera);
        if (!event.m_window.isCursorVisible())
        {
            EventDispatcher::dispatch<MouseMovedEvent>(event, &FirstPersonCamera::onMouseMoved, &m_camera);
        }
        EventDispatcher::dispatch<KeyPressedEvent>(event, [&](KeyPressedEvent const & event)
        {
            switch (event.m_key_code)
            {
            case GLFW_KEY_E:
                m_window.setCursorVisibility(!m_window.isCursorVisible());
                m_camera.setCursorPosition(event.m_window.getCursorPosition());
                break;
            case GLFW_KEY_F:
                m_window.setFullscreen(!m_window.isFullscreen());
                break;
            }
            m_world.onKeyPressed(event);
        });
        EventDispatcher::dispatch<MousePressedEvent>(event, [&](MousePressedEvent const & event)
        {
            switch (event.m_button_code)
            {
                case GLFW_MOUSE_BUTTON_LEFT:
                    m_world.m_create_destroy_multiplier = -1.0f;
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    m_world.m_create_destroy_multiplier = 1.0f;
                    break;
            }
        });
        EventDispatcher::dispatch<MouseScrolledEvent>(event, [&](MouseScrolledEvent const & event)
        {
            if (event.m_y_offset > 0) m_world.m_terraform_strength += 0.05f;
            else m_world.m_terraform_strength -= 0.05f;
        });
    }

    void Application::update(float delta_time)
    {
        glfwPollEvents();
        m_game_system.getGpuSynchronizer().update();
        if (!m_window.isCursorVisible()) m_world.update(delta_time, m_window, m_camera);
    }

    WorldGenerationConfig static config;
    bool static tweakable_lac_per = false;

    void Application::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_world.render(m_camera);

        if (m_window.isCursorVisible())
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            bool values_changed = false;

            values_changed |= ImGui::DragFloat("Chunk Size", &m_world.m_chunk_size_in_units, 0.1f, 0.0f, FLT_MAX);
            values_changed |= ImGui::DragFloat("Threshold", &m_world.m_threshold, 0.05f);
            ImGui::DragFloat("Terraform Radius", &m_world.m_terraform_radius, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Terraform Strength", &m_world.m_terraform_strength, 0.01f, -100.0f, 100.0f);

            if (values_changed |= ImGui::InputInt("Points/Chunk Axis", &m_world.m_points_per_axis, 1, 1))
            {
                m_world.setPointsPerAxis(m_world.m_points_per_axis);
            }
            
            if (values_changed |= ImGui::Checkbox("Tweakable Lacunarity/Persistence", &tweakable_lac_per))
            {
                if (!tweakable_lac_per)
                {
                    config.m_lacunarity_2d = 2.0f;
                    config.m_persistence_2d = 0.5f;
                    config.m_lacunarity_3d = 2.0f;
                    config.m_persistence_3d = 0.5f;
                }
            }

            ImGui::Separator();
            ImGui::Text("2D Noise");
            if (values_changed |= ImGui::InputInt("Octaves 2D", &config.m_octaves_2d, 1, 1))
            {
                if (config.m_octaves_2d < 1) config.m_octaves_2d = 1;
            }
            values_changed |= ImGui::DragFloat("Frequency 2D", &config.m_frequency_2d, 0.01f, 0.0f);
            values_changed |= ImGui::DragFloat("Amplitude 2D", &config.m_amplitude_2d, 0.01f, 0.0f);
            values_changed |= ImGui::DragFloat("Exponent 2D", &config.m_exponent_2d, 0.01f, 0.0f);

            if (tweakable_lac_per)
            {
                values_changed |= ImGui::DragFloat("Lacunarity 2D", &config.m_lacunarity_2d, 0.01f, 0.0f);
                values_changed |= ImGui::DragFloat("Persistence 2D", &config.m_persistence_2d, 0.01f, 0.0f);
            }

            ImGui::Separator();
            ImGui::Text("3D Noise");
            if (values_changed |= ImGui::InputInt("Octaves 3D", &config.m_octaves_3d, 1, 1))
            {
                if (config.m_octaves_3d < 1) config.m_octaves_3d = 1;
            }
            values_changed |= ImGui::DragFloat("Frequency 3D", &config.m_frequency_3d, 0.01f, 0.0f);
            values_changed |= ImGui::DragFloat("Amplitude 3D", &config.m_amplitude_3d, 0.01f, 0.0f);
            values_changed |= ImGui::DragFloat("Exponent 3D", &config.m_exponent_3d, 0.01f, 0.0f);

            if (tweakable_lac_per)
            {
                values_changed |= ImGui::DragFloat("Lacunarity 3D", &config.m_lacunarity_3d, 0.01f, 0.0f);
                values_changed |= ImGui::DragFloat("Persistence 3D", &config.m_persistence_3d, 0.01f, 0.0f);
            }

            if (values_changed)
            {
                m_world.updateGenerationConfig(config);
                m_world.invalidateAllChunks();
                m_world.generateChunks();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        m_textured_quad_shader->bind();
        m_textured_quad_shader->setUniformMatrix4f("u_model", glm::translate(glm::mat4(1.0f), glm::vec3(m_window.getWidth() / 2 - m_crosshair_texture->getWidth() / 2, m_window.getHeight() / 2 - m_crosshair_texture->getHeight() / 2, 0.0f)));
        m_textured_quad_shader->setUniformMatrix4f("u_projection", glm::ortho(0.0f, static_cast<float>(m_window.getWidth()), static_cast<float>(m_window.getHeight()), 0.0f));
        m_crosshair_texture->bind(0);
        glBindVertexArray(m_crosshair_va);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
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
                char frame_data[60];
                sprintf_s(frame_data, sizeof(frame_data), "FPS: %d, UPS: %d, Frametime: %f ms", frames, updates, frame_time * 1000.0);
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