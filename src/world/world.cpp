#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "world/simplex_noise.hpp"

#include "world/world.hpp"

namespace eng
{
    static constexpr inline int fastfloor(float fp)
    {
        int i = static_cast<int>(fp);
        return (fp < i) ? (i - 1) : (i);
    }

    World::World()
    {
        m_shader = std::make_shared<Shader>("res/shaders/shader.glsl");
        generateWorld(4, 4);
    }

    void World::generateWorld(unsigned int width, unsigned int height)
    {
        std::vector<float> vertices(width * height * 2);
        std::vector<int> indices((width - 1) * (height - 1) * 6);

        for (unsigned int i = 0; i < width * height; ++i)
        {
            vertices.push_back((float) (i % width));
            vertices.push_back((float) (i / width)); // Integer division
        }

        for (unsigned int i = 0; i < (width - 1) * (height - 1); ++i)
        {
            float skip_condition = (i + 1.0f) / width;
            if (skip_condition == fastfloor(skip_condition)) continue;
            indices.push_back(i);
            indices.push_back(i + width + 1);
            indices.push_back(i + width);

            indices.push_back(i);
            indices.push_back(i + 1);
            indices.push_back(i + width + 1);
        }

        m_ground_data = std::make_shared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(float), VertexBufferLayout{{{2, GL_FLOAT}}});
        for (int i : indices) ENG_LOG_F("%d", i);
        m_ground = std::make_shared<VertexArray>(indices.data(), indices.size() * sizeof(int));
        m_ground->setVertexBuffer(m_ground_data);
    }

    void World::render(FirstPersonCamera const & camera) const
    {
        m_shader->bind();
        m_shader->setUniformMatrix4f("u_model", glm::mat4(1.0f));
        m_shader->setUniformMatrix4f("u_view", camera.getViewMatrix());
        m_shader->setUniformMatrix4f("u_projection", camera.getProjectionMatrix());
        m_ground->bind();
        m_ground->drawElements();
    }
}