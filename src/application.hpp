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
        std::shared_ptr<Texture> m_crosshair_texture;
        std::shared_ptr<Shader> m_textured_quad_shader;
        std::shared_ptr<VertexArray> m_crosshair;
        std::shared_ptr<VertexBuffer> m_crosshair_quad_buffer;

    public:
        Application(unsigned int width, unsigned int height, char const * title, bool maximized);

        void run();
        void onEvent(Event const & event);
        void update(float delta_time);
        void render();
    };
}