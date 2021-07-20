#pragma once

#include <glm/glm.hpp>

#include "event/mouse_event.hpp"
#include "window.hpp"

namespace eng
{
    class FirstPersonCamera
    {
    private:
        glm::mat4 m_view_matrix, m_projection_matrix;
        glm::vec3 m_position{}, m_direction{}, m_right{}, m_up{};
        float m_fov{70}, m_aspect_ratio, m_yaw{}, m_pitch{}, m_last_x{}, m_last_y{};

        inline static float const SENSITIVITY = 0.0015f, MOVEMENT_SPEED = 1.5f;

    private:
        void calculateViewMatrix();
        void calculateProjectionMatrix();

    public:
        FirstPersonCamera(unsigned int viewport_width, unsigned int viewport_height);

        void update(float delta_time, Window const & window);

        void onMouseMoved(MouseMovedEvent const & event);

        void addPosition(glm::vec3 const & position);

        glm::mat4 const & getViewMatrix() const
        {
            return m_view_matrix;
        }

        glm::mat4 const & getProjectionMatrix() const
        {
            return m_projection_matrix;
        }
    };
}