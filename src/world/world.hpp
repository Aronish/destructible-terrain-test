#include <memory>

#include "graphics/vertex_array.hpp"
#include "graphics/vertex_buffer.hpp"
#include "graphics/shader.hpp"
#include "first_person_camera.hpp"

namespace eng
{
    class World
    {
    private:
        std::shared_ptr<VertexArray> m_ground;
        std::shared_ptr<VertexBuffer> m_ground_data;
        std::shared_ptr<Shader> m_shader;

    public:
        World();

        void generateWorld(unsigned int width, unsigned int height);

        void render(FirstPersonCamera const & camera) const;
    };
}