#include "event/key_event.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/vertex_buffer_layout.hpp"

#include "dc_app.hpp"

namespace eng
{
    DCApp::DCApp(int unsigned width, int unsigned height, char const * title, bool maximized) :
        m_window(width, height, title, maximized, std::bind(&DCApp::onEvent, this, std::placeholders::_1)),
        m_camera(width, height),
        m_cube(m_game_system, 1.0f),
        m_shader(m_game_system.getAssetManager().getShader("res/shaders/simple.glsl"))
    {
        glfwSwapInterval(1);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.79f, 0.94f, 1.0f, 1.0f);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_root.addContent({ 2.3f, 2.2f, 2.6f });
        m_root.addContent({ 7.3f, 13.7f, 6.3f });
    }

    void DCApp::onEvent(Event const & event)
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
        });
    }

    void DCApp::update(float delta_time)
    {
        glfwPollEvents();
        m_game_system.getGpuSynchronizer().update();
        if (!m_window.isCursorVisible())
        {
            float speed = WALKING_SPEED * (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_LEFT_SHIFT) ? RUN_MULTIPLIER : 1.0f);
            float cos_yaw = std::cosf(m_camera.getYaw()), sin_yaw = std::sinf(m_camera.getYaw());
            glm::vec2 direction{};
            float y_velocity{};
            bool moved{};

            if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_W)) direction += glm::vec2{ cos_yaw, sin_yaw };
            if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_A)) direction += glm::mat2(0.0f, -1.0f, 1.0f, 0.0f) * glm::vec2{ cos_yaw, sin_yaw };
            if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_S)) direction += -glm::vec2{ cos_yaw, sin_yaw };
            if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_D)) direction += glm::mat2(0.0f, 1.0f, -1.0f, 0.0f) * glm::vec2{ cos_yaw, sin_yaw };
            if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_SPACE)) y_velocity = speed;
            else if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_C)) y_velocity = -speed;
            if (glm::length(direction) > 0.0f)
            {
                direction = glm::normalize(direction) * speed;
                moved = true;
            }
            if (y_velocity != 0.0f) moved = true;
            if (moved) m_camera.addPosition(glm::vec3{ direction.x, y_velocity, direction.y } * delta_time);
        }
    }

    void DCApp::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_shader->bind();
        m_shader->setUniformFloat("u_color", 0.0f);
        m_shader->setUniformMatrix4f("u_view", m_camera.getViewMatrix());
        m_shader->setUniformMatrix4f("u_projection", m_camera.getProjectionMatrix());
        glBindVertexArray(m_cube.m_va);
        m_root.render(m_shader, m_cube);
        glfwSwapBuffers(m_window.getWindowHandle());
    }

    void DCApp::run()
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