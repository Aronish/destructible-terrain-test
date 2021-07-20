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

namespace eng
{
    class Application
    {
    private:
        std::shared_ptr<VertexArray> m_vao;
        std::shared_ptr<VertexBuffer> m_vbo;
        std::shared_ptr<Shader> m_shader;
        std::shared_ptr<Texture> m_texture;
        FirstPersonCamera m_camera;

    public:
        Application(unsigned int width, unsigned int height, char const * title);

        void run();
        void onEvent(Event const & event);
        void update(float delta_time);
        void render();

    private:
        Window m_window;
    };
}