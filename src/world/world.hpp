#pragma once

#include <memory>
#include <unordered_map>

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
        friend class Application;
    public:
        int unsigned static constexpr WORK_GROUP_SIZE = 8;
    private:
        int unsigned static inline s_resolution = 4, s_points_per_axis = s_resolution * WORK_GROUP_SIZE, s_max_triangle_amount = (s_points_per_axis - 1) * (s_points_per_axis - 1) * (s_points_per_axis - 1) * 4;

    private:
        int m_octaves = 1;
        float m_surface_level = 0.5f, m_frequency = 0.1f, m_amplitude = 1.0f, m_lacunarity = 2.0f, m_persistence = 1.0f / m_lacunarity;

        std::shared_ptr<Shader> s_density_generator;
        std::shared_ptr<Shader> s_marching_cubes;
        std::shared_ptr<Shader> s_chunk_renderer;
        std::shared_ptr<ShaderStorageBuffer> s_isosurface;
        std::shared_ptr<ShaderStorageBuffer> s_triangulation_table;
        std::shared_ptr<AtomicCounterBuffer> s_triangle_counter;

        std::unordered_map<uint64_t, Chunk> m_chunk_map;

    public:
        void onRendererInit(AssetManager const & asset_manager);

        void initDependentBuffers();
        
        void setResolution(int unsigned resolution);

        void generateChunks();

        void render(FirstPersonCamera const & camera) const;

        int unsigned static getPointsPerAxis();
    };
}