#include <numbers>

#include <glm/gtc/matrix_transform.hpp>

#include "first_person_camera.hpp"

namespace eng
{
    FirstPersonCamera::FirstPersonCamera(unsigned int viewport_width, unsigned int viewport_height) : m_aspect_ratio((float)viewport_width / (float)viewport_height)
    {
        calculateViewMatrix();
        calculateProjectionMatrix();
    }

    void FirstPersonCamera::update(float delta_time, Window const & window)
    {
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_W))
        {
            addPosition(m_direction * delta_time * MOVEMENT_SPEED);
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_S))
        {
            addPosition(m_direction * -delta_time * MOVEMENT_SPEED);
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_D))
        {
            addPosition(m_right * delta_time * MOVEMENT_SPEED);
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_A))
        {
            addPosition(m_right * -delta_time * MOVEMENT_SPEED);
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_SPACE))
        {
            addPosition(m_up * delta_time * MOVEMENT_SPEED);
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_C))
        {
            addPosition(m_up * -delta_time * MOVEMENT_SPEED);
        }
    }

    void FirstPersonCamera::onMouseMoved(MouseMovedEvent const & event)
    {
        m_yaw += ((float)event.m_x_pos - m_last_x) * SENSITIVITY;
        m_pitch += (m_last_y - (float)event.m_y_pos) * SENSITIVITY;
        float almost_half_pi = std::numbers::pi_v<float> / 2.0f - 0.001f;
        if (m_pitch < -almost_half_pi) m_pitch = -almost_half_pi;
        if (m_pitch > almost_half_pi) m_pitch = almost_half_pi;
        m_last_x = (float)event.m_x_pos;
        m_last_y = (float)event.m_y_pos;
        calculateViewMatrix();
    }

    void FirstPersonCamera::onWindowResized(WindowResizedEvent const & event)
    {
        m_aspect_ratio = (float) event.m_width / (float) event.m_height;
        calculateProjectionMatrix();
    }

    void FirstPersonCamera::setPosition(glm::vec3 const & position)
    {
        m_position = position;
        calculateViewMatrix();
    }

    void FirstPersonCamera::addPosition(glm::vec3 const & position)
    {
        m_position += position;
        calculateViewMatrix();
    }

    void FirstPersonCamera::calculateViewMatrix()
    {
        double cos_pitch    = std::cos(m_pitch);
        m_direction         = glm::normalize(glm::vec3{ std::cos(m_yaw) * cos_pitch, std::sin(m_pitch), std::sin(m_yaw) * cos_pitch });
        m_right             = glm::normalize(glm::cross(m_direction, { 0.0f, 1.0f, 0.0f }));
        m_up                = glm::normalize(glm::cross(m_right, m_direction));
        m_view_matrix       = glm::lookAt(m_position, m_position + m_direction, { 0.0f, 1.0f, 0.0f });
    }

    void FirstPersonCamera::calculateProjectionMatrix()
    {
        m_projection_matrix = glm::perspective(m_fov, m_aspect_ratio, 0.1f, 100.0f);
    }
}