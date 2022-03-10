#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "event/event.hpp"
#include "first_person_camera.hpp"
#include "graphics/asset.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "graphics/vertex_buffer.hpp"
#include "graphics/vertex_array.hpp"
#include "window.hpp"
#include "world/world.hpp"
#include "world/chunk.hpp"

namespace eng
{
    class Application
    {
    private:
        Window m_window; // Has to be first due to OpenGL initialization
        AssetManager m_asset_manager;
        FirstPersonCamera m_camera;
        World m_world;

        glm::vec2 m_last_chunk_coords;
        int m_render_distance = 3;

    public:
        Application(unsigned int width, unsigned int height, char const * title);

        void run();
        void onEvent(Event const & event);
        void update(float delta_time);
        void render();
    };
}