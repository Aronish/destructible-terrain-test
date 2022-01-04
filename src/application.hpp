#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "event/event.hpp"
#include "graphics/vertex_buffer.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "first_person_camera.hpp"
#include "window.hpp"
#include "world/world.hpp"
#include "world/cs_world.hpp"

namespace eng
{
    class Application
    {
    private:
        Window m_window; // Has to be first due to OpenGL initialization
        FirstPersonCamera m_camera;
        ComputeWorld m_compute_world;

        float m_step_size = 0.05f;
        int m_grid_size = 2;

    public:
        Application(unsigned int width, unsigned int height, char const * title);

        void run();
        void onEvent(Event const & event);
        void update(float delta_time);
        void render();
    };
}