#pragma once

#include <memory>
#include <unordered_map>

#include <glad/glad.h>

#include "first_person_camera.hpp"
#include "graphics/asset.hpp"
#include "graphics/atomic_counter_buffer.hpp"
#include "graphics/shader.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/vertex_buffer.hpp"
#include "world/chunk.hpp"

namespace eng
{
    class World
    {
        friend class Chunk;
        friend class Application; //For accessing these statics in GUI
    public:
        int unsigned static constexpr WORK_GROUP_SIZE = 8;
    private:
        int unsigned static inline s_resolution = 1, s_points_per_axis = s_resolution * WORK_GROUP_SIZE, s_max_triangle_amount = (s_points_per_axis - 1) * (s_points_per_axis - 1) * (s_points_per_axis - 1) * 4;

    private:
        int m_octaves = 1, m_render_distance = 2;
        float m_surface_level = 0.5f, m_frequency = 0.1f, m_amplitude = 1.0f, m_lacunarity = 2.0f, m_persistence = 1.0f / m_lacunarity;

        std::shared_ptr<Shader> m_density_generator;
        std::shared_ptr<Shader> m_marching_cubes;
        std::shared_ptr<Shader> m_chunk_renderer;
        std::shared_ptr<ShaderStorageBuffer> m_isosurface;
        std::shared_ptr<ShaderStorageBuffer> m_triangulation_table;
        std::shared_ptr<AtomicCounterBuffer> m_triangle_counter;

        GLuint m_max_chunk_index_buffer;
        int unsigned * m_counter_buffer;

        ChunkPool m_chunk_pool;

    public:
        ~World();

        void onRendererInit(AssetManager const & asset_manager);

        void initDependentBuffers();
        
        void setResolution(int unsigned resolution);

        void setRenderDistance(int unsigned render_distance);

        void generateChunks(glm::ivec2 const & origin);

        void render(FirstPersonCamera const & camera) const;

        int unsigned static getPointsPerAxis();
    };
}