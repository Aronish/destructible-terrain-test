#include <functional>

#include "event/key_event.hpp"
#include "logger.hpp"
#include "graphics/vertex_array.hpp"

#include "dc_app.hpp"

namespace eng
{
    DCApplication::DCApplication()
        : m_window(1280, 720, "Dual Contouring", false, std::bind(&DCApplication::onEvent, this, std::placeholders::_1)),
        m_camera(m_window.getWidth(), m_window.getHeight()),
        m_plane(m_game_system)
	{
        glfwSwapInterval(1);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(0.79f, 0.94f, 1.0f, 1.0f);
        glPointSize(4.0f);

        m_quad_shader = m_game_system.getAssetManager().getShader("res/shaders/simple.glsl");

        m_camera.setPosition({ -1.0f, 0.0f, -1.0f });
        m_window.setCursorVisibility(false);
	}

	void DCApplication::onEvent(Event const & event)
	{
        if (!m_window.isCursorVisible()) EventDispatcher::dispatch<MouseMovedEvent>(event, &FirstPersonCamera::onMouseMoved, &m_camera);
        EventDispatcher::dispatch<WindowResizedEvent>(event, &FirstPersonCamera::onWindowResized, &m_camera);
        EventDispatcher::dispatch<KeyPressedEvent>(event, [this](KeyPressedEvent const & event)
        {
            if (event.m_key_code == GLFW_KEY_E) m_window.setCursorVisibility(!m_window.isCursorVisible());
            if (event.m_key_code == GLFW_KEY_R)
            {
                m_plane.generate();
            }
        });
	}

	void DCApplication::update(float delta_time)
	{
		glfwPollEvents();
		m_game_system.getGpuSynchronizer().update();
        glm::vec3 const right = glm::normalize(glm::cross({ 0.0f, 1.0f, 0.0f }, m_camera.getDirection()));
        if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_W)) m_camera.addPosition(m_camera.getDirection() * delta_time);
        if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_S)) m_camera.addPosition(m_camera.getDirection() * -delta_time);
        if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_A)) m_camera.addPosition(right * delta_time);
        if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_D)) m_camera.addPosition(right * -delta_time);
        if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_SPACE)) m_camera.addPosition(glm::vec3{ 0.0f, 1.0f, 0.0f } * delta_time);
        if (glfwGetKey(m_window.getWindowHandle(), GLFW_KEY_C)) m_camera.addPosition(glm::vec3{ 0.0f, -1.0f, 0.0f } * delta_time);
	}

	void DCApplication::render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_quad_shader->bind();
        m_quad_shader->setUniformMatrix4f("u_model", glm::scale(glm::mat4(1.0f), glm::vec3{ 0.2f }));
        m_quad_shader->setUniformMatrix4f("u_view", m_camera.getViewMatrix());
        m_quad_shader->setUniformMatrix4f("u_projection", m_camera.getProjectionMatrix());
        m_quad_shader->setUniformVector4f("u_color", { 0.8f, 0.2f, 0.3f, 1.0f });
        glBindVertexArray(m_plane.m_va);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_plane.m_di);
        glDrawArraysIndirect(GL_TRIANGLES, nullptr);
		glfwSwapBuffers(m_window.getWindowHandle());
	}

	void DCApplication::run()
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