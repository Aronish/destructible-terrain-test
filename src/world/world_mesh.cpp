#include "world.hpp"

namespace eng
{
    int constexpr sign(float x)
    {
        return (x > 0) - (x < 0);
    }

    float constexpr sqr(float x)
    {
        return x * x;
    }

    bool sphereCubeIntersect(glm::vec3 const & cube_min, glm::vec3 const & cube_max, glm::vec4 const & sphere_pos_radius)
    {
        float dist_squared = sqr(sphere_pos_radius.w);
        if (sphere_pos_radius.x < cube_min.x) dist_squared -= sqr(sphere_pos_radius.x - cube_min.x);
        else if (sphere_pos_radius.x > cube_max.x) dist_squared -= sqr(sphere_pos_radius.x - cube_max.x);
        if (sphere_pos_radius.y < cube_min.y) dist_squared -= sqr(sphere_pos_radius.y - cube_min.y);
        else if (sphere_pos_radius.y > cube_max.y) dist_squared -= sqr(sphere_pos_radius.y - cube_max.y);
        if (sphere_pos_radius.z < cube_min.z) dist_squared -= sqr(sphere_pos_radius.z - cube_min.z);
        else if (sphere_pos_radius.z > cube_max.z) dist_squared -= sqr(sphere_pos_radius.z - cube_max.z);
        return dist_squared > 0;
    }

    void World::castRay(FirstPersonCamera const & camera)
    {
        // 3D voxel traversal through chunks along ray
        int dx = sign(camera.getDirection().x), dy = sign(camera.getDirection().y), dz = sign(camera.getDirection().z);

        float t_delta_x = 0.0f, t_max_x = 0.0f, adjusted_position_x = camera.getPosition().x / m_chunk_size_in_units;
        if (dx != 0) t_delta_x = std::fmin(m_chunk_size_in_units * dx / camera.getDirection().x, 10'000'000.0f); else t_delta_x = 10'000'000.0f;
        if (dx > 0) t_max_x = t_delta_x * (1 - adjusted_position_x + std::floorf(adjusted_position_x)); else t_max_x = t_delta_x * (adjusted_position_x - std::floorf(adjusted_position_x));
        int x = m_last_chunk_coords.x;

        float t_delta_y = 0.0f, t_max_y = 0.0f, adjusted_position_y = camera.getPosition().y / m_chunk_size_in_units;
        if (dy != 0) t_delta_y = std::fmin(m_chunk_size_in_units * dy / camera.getDirection().y, 10'000'000.0f); else t_delta_y = 10'000'000.0f;
        if (dy > 0) t_max_y = t_delta_y * (1 - adjusted_position_y + std::floorf(adjusted_position_y)); else t_max_y = t_delta_y * (adjusted_position_y - std::floorf(adjusted_position_y));
        int y = m_last_chunk_coords.y;

        float t_delta_z = 0.0f, t_max_z = 0.0f, adjusted_position_z = camera.getPosition().z / m_chunk_size_in_units;
        if (dz != 0) t_delta_z = std::fmin(m_chunk_size_in_units * dz / camera.getDirection().z, 10'000'000.0f); else t_delta_z = 10'000'000.0f;
        if (dz > 0) t_max_z = t_delta_z * (1 - adjusted_position_z + std::floorf(adjusted_position_z)); else t_max_z = t_delta_z * (adjusted_position_z - std::floorf(adjusted_position_z));
        int z = m_last_chunk_coords.z;

        for (int i = 0; i < RAY_HIT_DATA_SIZE; ++i) m_hit_info_ptr[i] = 0; // Reset hit info
        int const raycast_reach = 1;
        // Don't bother reading back hit info and stopping on hit, it's faster to just check every possibility
        while (std::abs(x - m_last_chunk_coords.x) <= raycast_reach && std::abs(z - m_last_chunk_coords.z) <= raycast_reach && y >= 0 && y < 2)
        {
            chunkRayIntersection(glm::ivec3{ x, y, z }, camera.getPosition(), camera.getDirection());
            if (t_max_x < t_max_y)
            {
                if (t_max_x < t_max_z)
                {
                    t_max_x += t_delta_x;
                    x += dx;
                }
                else
                {
                    t_max_z += t_delta_z;
                    z += dz;
                }
            }
            else
            {
                if (t_max_y < t_max_z)
                {
                    t_max_y += t_delta_y;
                    y += dy;
                }
                else
                {
                    t_max_z += t_delta_z;
                    z += dz;
                }
            }
        }
        r_game_system.getGpuSynchronizer().setBarrier([this, sphereCubeIntersect = sphereCubeIntersect] // Terraforming deferred to when raycast is finished
        {
            if (m_hit_info_ptr[18])
            {
                glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_generation_config_u);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_triangulation_table_ss);

                for (int i = 0; i < 18; ++i)
                {
                    int x = i % 3 - 1, y = i / 9, z = i / 3 % 3 - 1;
                    if (sphereCubeIntersect({ x, y, z }, glm::ivec3{ x, y, z } + 1, { m_hit_info_ptr[0], m_hit_info_ptr[1], m_hit_info_ptr[2], m_terraform_radius / m_points_per_axis + 0.1f })) // Intersection test in unit space
                    {
                        terraform({ m_hit_info_ptr[19] + x, m_hit_info_ptr[20] + y, m_hit_info_ptr[21] + z });
                        // CHUNK (0, 0) DOES SOMETHING WEIRD
                    }
                }
            }
        });
    }

    void World::chunkRayIntersection(glm::ivec3 const & chunk_coordinate, glm::vec3 const & origin, glm::vec3 const & direction)
    {
        std::vector<Chunk>::iterator current_chunk;
        if (!m_chunk_pool.getChunkAt(chunk_coordinate, current_chunk)) return;
        // Generate dispatch command based on amount of triangles in chunk
        m_ray_mesh_command->bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_dispatch_indirect_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, current_chunk->getDrawIndirectBuffer());
        glDispatchCompute(1, 1, 1);

        glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(m_chunk_size_in_units)) * glm::translate(glm::mat4(1.0f), static_cast<glm::vec3>(current_chunk->getPosition()));
        // Setup raycast input/output
        m_mesh_ray_intersect->bind();
        m_mesh_ray_intersect->setUniformMatrix4f("u_transform", transform);
        m_mesh_ray_intersect->setUniformVector3f("u_chunk_coordinate", static_cast<glm::vec3>(current_chunk->getPosition()));
        m_mesh_ray_intersect->setUniformVector3f("u_ray_origin", origin);
        m_mesh_ray_intersect->setUniformVector3f("u_ray_direction", direction);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, current_chunk->getMeshVB());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ray_hit_data_ss);
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, m_dispatch_indirect_buffer);
        glDispatchComputeIndirect(0);
    }

    void World::generateMesh(Chunk * chunk, glm::ivec3 const & chunk_coordinate, uint8_t has_neighbors)
    {
        m_marching_cubes->bind();
        m_marching_cubes->setUniformFloat("u_threshold", m_threshold);
        m_marching_cubes->setUniformInt("u_points_per_axis", m_points_per_axis);
        m_marching_cubes->setUniformInt("u_has_neighbors", has_neighbors);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, chunk->getMeshVB());
        glClearNamedBufferData(chunk->getMeshVB(), GL_R32F, GL_RED, GL_FLOAT, nullptr);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, chunk->getDrawIndirectBuffer());
        glNamedBufferSubData(chunk->getDrawIndirectBuffer(), 0, sizeof(INITIAL_INDIRECT_DRAW_CONFIG), INITIAL_INDIRECT_DRAW_CONFIG);
        bindNeighborChunks(4, has_neighbors, chunk_coordinate);
        m_marching_cubes->dispatchCompute(m_resolution, m_resolution, m_resolution);
    }

    void World::terraform(glm::ivec3 const & chunk_coordinate)
    {
        std::vector<Chunk>::iterator chunk;
        if (m_chunk_pool.getChunkAt(chunk_coordinate, chunk))
        {
            m_terraform->bind();
            m_terraform->setUniformInt("u_points_per_axis", m_points_per_axis);
            m_terraform->setUniformFloat("u_strength", m_terraform_strength * m_create_destroy_multiplier);
            m_terraform->setUniformFloat("u_radius", m_terraform_radius);
            m_terraform->setUniformVector3f("u_current_chunk", static_cast<glm::vec3>(chunk_coordinate));
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, chunk->getDensityDistributionBuffer());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ray_hit_data_ss);
            m_terraform->dispatchCompute(m_resolution, m_resolution, m_resolution);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            uint8_t has_neighbors = (chunk_coordinate.x != m_last_chunk_coords.x - m_render_distance) | ((chunk_coordinate.z != m_last_chunk_coords.z - m_render_distance) << 1) | ((chunk_coordinate.y == 1) << 2);

            generateMesh(&(*chunk), chunk_coordinate, has_neighbors);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            glFlush();
        }
    }
}