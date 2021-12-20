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
        std::shared_ptr<VertexArray> m_vertex_array;
        std::shared_ptr<VertexBuffer> m_vertex_buffer;
        std::shared_ptr<Shader> m_shader;

    public:
        World();

        void generateWorld(float grid_size, float step_size, float surface_level);

        void render(FirstPersonCamera const & camera) const;
    };
}