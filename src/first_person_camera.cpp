#include <numbers>

#include <glm/gtc/matrix_transform.hpp>

#include "logger.hpp"

#include "first_person_camera.hpp"

namespace eng
{
    FirstPersonCamera::FirstPersonCamera(unsigned int viewport_width, unsigned int viewport_height) : m_aspect_ratio((float)viewport_width / (float)viewport_height)
    {
        calculateViewMatrix();
        calculateProjectionMatrix();
    }

    void FirstPersonCamera::calculateViewMatrix()
    {
        double cos_pitch = std::cos(m_pitch);
        m_direction = glm::normalize(glm::vec3{ std::cos(m_yaw) * cos_pitch, std::sin(m_pitch), std::sin(m_yaw) * cos_pitch });
        m_right = glm::normalize(glm::cross(m_direction, { 0.0f, 1.0f, 0.0f }));
        m_up = glm::normalize(glm::cross(m_right, m_direction));
        m_view_matrix = glm::lookAt(m_position, m_position + m_direction, { 0.0f, 1.0f, 0.0f });
    }

    void FirstPersonCamera::calculateProjectionMatrix()
    {
        m_projection_matrix = glm::perspective(m_fov, m_aspect_ratio, 0.1f, 1000.0f);
    }

    void FirstPersonCamera::onMouseMoved(MouseMovedEvent const & event)
    {
        m_yaw += ((float)event.m_x_pos - m_last_x) * SENSITIVITY;
        if (m_yaw >= 2.0f * std::numbers::pi_v<float>) m_yaw = 0.0f;
        if (m_yaw < 0.0f) m_yaw = 2.0f * std::numbers::pi_v<float>;
        m_pitch += (m_last_y - (float)event.m_y_pos) * SENSITIVITY;
        float almost_half_pi = std::numbers::pi_v<float> / 2.0f - 0.001f;
        if (m_pitch < -almost_half_pi) m_pitch = -almost_half_pi;
        if (m_pitch > almost_half_pi) m_pitch = almost_half_pi;
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

    void FirstPersonCamera::setCursorPosition(std::pair<double, double> position)
    {
        m_last_x = (float)position.first;
        m_last_y = (float)position.second;
    }

    float FirstPersonCamera::getYaw() const
    {
        return m_yaw;
    }

    glm::vec3 const & FirstPersonCamera::getPosition() const
    {
        return m_position;
    }

    glm::vec3 const & FirstPersonCamera::getDirection() const
    {
        return m_direction;
    }

    glm::mat4 const & FirstPersonCamera::getViewMatrix() const
    {
        return m_view_matrix;
    }

    glm::mat4 const & FirstPersonCamera::getProjectionMatrix() const
    {
        return m_projection_matrix;
    }
}