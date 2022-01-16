#include <memory>

#include "graphics/vertex_array.hpp"
#include "graphics/vertex_buffer.hpp"
#include "graphics/shader.hpp"
#include "graphics/shader_storage_buffer.hpp"
#include "graphics/atomic_counter_buffer.hpp"
#include "first_person_camera.hpp"

namespace eng
{
    class ComputeWorld
    {
    public:
        int unsigned static constexpr WORK_GROUP_SIZE = 8;

        float m_surface_level = 0.5f;
        int unsigned m_resolution = 1, m_points_per_axis = m_resolution * WORK_GROUP_SIZE, m_max_triangle_amount = (m_points_per_axis - 1) * (m_points_per_axis - 1) * (m_points_per_axis - 1) * 4;

    private:
#pragma warning(push)
#pragma warning(disable : 4324)
        struct GLSLTriangle
        {
            alignas(16) glm::vec3 vertexA, vertexB, vertexC, normal;
        };
#pragma warning(pop)

        bool m_mesh_empty = true;

        std::shared_ptr<VertexArray> m_vertex_array;
        std::shared_ptr<VertexBuffer> m_vertex_buffer;
        std::shared_ptr<Shader> m_visualization_shader;

        std::shared_ptr<Shader> m_generate_points;
        std::shared_ptr<ShaderStorageBuffer> m_values;

        std::shared_ptr<AtomicCounterBuffer> m_triangle_counter;
        std::shared_ptr<Shader> m_marching_cubes;
        std::shared_ptr<ShaderStorageBuffer> m_triangles;
        std::shared_ptr<ShaderStorageBuffer> m_triangulation_table;

    public:
        ComputeWorld();

        void initializeBuffers();

        void generateWorld();

        void setResolution(int unsigned resolution);

        void visualize(FirstPersonCamera const & camera) const;
    };
}