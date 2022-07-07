#include <array>
#include <chrono>
#include <cmath>

#include "glm/gtc/type_ptr.hpp"

#include "world/world.hpp"

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
        if      (sphere_pos_radius.x < cube_min.x) dist_squared -= sqr(sphere_pos_radius.x - cube_min.x);
        else if (sphere_pos_radius.x > cube_max.x) dist_squared -= sqr(sphere_pos_radius.x - cube_max.x);
        if      (sphere_pos_radius.y < cube_min.y) dist_squared -= sqr(sphere_pos_radius.y - cube_min.y);
        else if (sphere_pos_radius.y > cube_max.y) dist_squared -= sqr(sphere_pos_radius.y - cube_max.y);
        if      (sphere_pos_radius.z < cube_min.z) dist_squared -= sqr(sphere_pos_radius.z - cube_min.z);
        else if (sphere_pos_radius.z > cube_max.z) dist_squared -= sqr(sphere_pos_radius.z - cube_max.z);
        return dist_squared > 0;
    }

    World::World(GameSystem & game_system) : r_game_system(game_system), m_chunk_pool(game_system)
    {
        m_density_generator =   game_system.getAssetManager().getShader("res/shaders/generate_points.glsl");
        m_marching_cubes =      game_system.getAssetManager().getShader("res/shaders/marching_cubes.glsl");
        m_chunk_renderer =      game_system.getAssetManager().getShader("res/shaders/light_test.glsl");
        m_mesh_ray_intersect =  game_system.getAssetManager().getShader("res/shaders/mesh_ray_intersect.glsl");
        m_ray_mesh_command =    game_system.getAssetManager().getShader("res/shaders/ray_mesh_command.glsl");
        m_terraform =           game_system.getAssetManager().getShader("res/shaders/terraform.glsl");

        auto tri_table = new int[256][16]
        {
            { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
            { 8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1 },
            { 3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1 },
            { 4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
            { 4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
            { 5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1 },
            { 9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1 },
            { 10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1 },
            { 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
            { 5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1 },
            { 10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1 },
            { 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1 },
            { 2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
            { 7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
            { 2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1 },
            { 11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1 },
            { 5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1 },
            { 11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1 },
            { 11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
            { 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1 },
            { 2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
            { 5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1 },
            { 6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
            { 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1 },
            { 6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
            { 5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
            { 10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
            { 6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1 },
            { 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1 },
            { 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1 },
            { 3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
            { 5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
            { 0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1 },
            { 9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1 },
            { 8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
            { 5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1 },
            { 0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1 },
            { 6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1 },
            { 10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1 },
            { 10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
            { 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
            { 1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1 },
            { 0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
            { 10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1 },
            { 3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
            { 6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1 },
            { 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1 },
            { 8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1 },
            { 3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
            { 6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1 },
            { 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1 },
            { 10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
            { 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1 },
            { 7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
            { 7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
            { 2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1 },
            { 1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1 },
            { 11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1 },
            { 8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1 },
            { 0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1 },
            { 7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
            { 10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
            { 6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1 },
            { 7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1 },
            { 10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
            { 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1 },
            { 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1 },
            { 7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
            { 6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
            { 8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1 },
            { 6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1 },
            { 4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1 },
            { 10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1 },
            { 8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1 },
            { 1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
            { 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1 },
            { 10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1 },
            { 10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
            { 5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
            { 11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1 },
            { 9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
            { 6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1 },
            { 7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1 },
            { 3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1 },
            { 7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1 },
            { 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1 },
            { 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1 },
            { 9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1 },
            { 1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1 },
            { 4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1 },
            { 7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1 },
            { 6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1 },
            { 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1 },
            { 6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1 },
            { 0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1 },
            { 11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1 },
            { 6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1 },
            { 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1 },
            { 9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1 },
            { 1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1 },
            { 10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1 },
            { 0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1 },
            { 5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1 },
            { 10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1 },
            { 11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1 },
            { 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1 },
            { 7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1 },
            { 2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
            { 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1 },
            { 9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1 },
            { 9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1 },
            { 1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
            { 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1 },
            { 0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1 },
            { 10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1 },
            { 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1 },
            { 0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1 },
            { 0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1 },
            { 9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1 },
            { 5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1 },
            { 5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1 },
            { 8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1 },
            { 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1 },
            { 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1 },
            { 3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1 },
            { 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1 },
            { 9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1 },
            { 11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
            { 11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1 },
            { 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1 },
            { 9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1 },
            { 3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1 },
            { 1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1 },
            { 4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1 },
            { 0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
            { 9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1 },
            { 1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { 0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
            { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
        };

        m_triangulation_table_ss = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_triangulation_table_ss, sizeof(int) * 256 * 16, tri_table, 0);

        refreshGenerationSpec();
        size_t config_buffer_size{};
        for (auto const & block_variable : m_generation_spec) config_buffer_size += GLTypeToSize(block_variable.m_type);

        m_generation_config_u = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_generation_config_u, config_buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);

        m_ray_hit_data_ss = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_ray_hit_data_ss, sizeof(float) * RAY_HIT_DATA_SIZE, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        m_hit_info_ptr = reinterpret_cast<float *>(glMapNamedBuffer(m_ray_hit_data_ss, GL_READ_WRITE));

        m_chunk_va = game_system.getAssetManager().createVertexArray();
        VertexArray::setVertexArrayFormat(m_chunk_va, VertexDataLayout::POSITION_NORMAL_3F);

        m_dispatch_indirect_buffer = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_dispatch_indirect_buffer, sizeof(int unsigned) * 3, nullptr, GL_DYNAMIC_STORAGE_BIT);

        m_chunk_collider_material = game_system.getPhysx()->createMaterial(1.0f, 1.0f, 1.0f);

        physx::PxSceneDesc scene_desc{game_system.getPhysx()->getTolerancesScale()};
        scene_desc.gravity = physx::PxVec3{ 0.0f, -9.81f, 0.0f };
        if (!scene_desc.cpuDispatcher) scene_desc.cpuDispatcher = r_game_system.getPhysxCpuDispatcher();
        if (!scene_desc.filterShader) scene_desc.filterShader = physx::PxDefaultSimulationFilterShader;
        m_scene = game_system.getPhysx()->createScene(scene_desc);
        if (!m_scene) throw std::runtime_error("Failed to create PhysX scene!");

        m_controller_manager = PxCreateControllerManager(*m_scene);
        m_player.initCharacterController(m_controller_manager, game_system, { 0.0f, 15.0f, 0.0f });

        m_chunk_pool.initialize(static_cast<size_t>((2 * m_render_distance + 1) * (2 * m_render_distance + 1) * 2), m_max_triangle_count, m_points_per_axis);
        for (auto const & chunk : m_chunk_pool)
        {
            m_scene->addActor(*chunk.getRigidBody());
        }

        initDynamicBuffers();

        delete[] tri_table;
    }
    
    World::~World()
    {
        m_controller_manager->release();
        m_scene->release();
        m_chunk_collider_material->release();
    }

    void World::initDynamicBuffers()
    {
        m_chunk_pool.setMeshConfig(m_max_triangle_count, m_points_per_axis);
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
            chunkRayIntersection(glm::ivec3{x, y, z}, camera.getPosition(), camera.getDirection());
            if (t_max_x < t_max_y)
            {
                if (t_max_x < t_max_z)
                {
                    t_max_x += t_delta_x;
                    x += dx;
                } else
                {
                    t_max_z += t_delta_z;
                    z += dz;
                }
            } else
            {
                if (t_max_y < t_max_z)
                {
                    t_max_y += t_delta_y;
                    y += dy;
                } else
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

    void World::debugRecompile()
    {
        m_density_generator->compile("res/shaders/generate_points.glsl");
        m_chunk_renderer->compile("res/shaders/light_test.glsl");
        m_marching_cubes->compile("res/shaders/marching_cubes.glsl");

        refreshGenerationSpec();
        invalidateAllChunks();
        generateChunks();
    }

    void World::onPlayerMoved(glm::vec3 const & position)
    {
        auto chunk_coords = static_cast<glm::ivec3>(glm::floor(position / static_cast<float>(m_chunk_size_in_units)));
        if (m_last_chunk_coords != chunk_coords)
        {
            m_last_chunk_coords = chunk_coords;
            generateChunks();
        }
    }

    void World::invalidateAllChunks()
    {
        for (auto & chunk : m_chunk_pool)
        {
            m_chunk_pool.deactivateChunk(&chunk);
        }
    }
    
    void World::bindNeighborChunks(int unsigned starting_index, uint8_t neighbor_mask, glm::ivec3 const & chunk_coordinate)
    {
        for (uint8_t i = 1; i <= 7; ++i)
        {
            if ((neighbor_mask & i) == i)
            {
                std::vector<Chunk>::iterator neighbor;
                m_chunk_pool.getChunkAt(chunk_coordinate - glm::ivec3{ (i & 0b001) == 0b001, (i & 0b100) == 0b100, (i & 0b010) == 0b010 }, neighbor);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, starting_index + (i - 1), neighbor->getDensityDistributionBuffer());
            }
        }
    }

    void World::generateChunks()
    {
        // Deactivate chunks out of render distance
        for (auto & chunk : m_chunk_pool)
        {
            if (!chunk.isActive()) continue;
            if (std::abs(chunk.getPosition().x - m_last_chunk_coords.x) > m_render_distance || std::abs(chunk.getPosition().z - m_last_chunk_coords.z) > m_render_distance)
            {
                m_chunk_pool.deactivateChunk(&chunk);
            }
        }
        // Generate chunks in render distance if they're not active
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_generation_config_u);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_triangulation_table_ss);
        for (int x_i = -m_render_distance; x_i <= m_render_distance; ++x_i)
        {
            for (int z_i = -m_render_distance; z_i <= m_render_distance; ++z_i)
            {
                for (int y_i = 0; y_i < 2; ++y_i)
                {
                    glm::ivec3 chunk_coordinate{ x_i + m_last_chunk_coords.x, y_i, z_i + m_last_chunk_coords.z };
                    if (!m_chunk_pool.hasChunkAt(chunk_coordinate))
                    {
                        Chunk * chunk = nullptr;
                        if (!m_chunk_pool.activateChunk(chunk, chunk_coordinate, m_chunk_size_in_units)) continue;
                        uint8_t has_neighbors = (x_i != -m_render_distance) | ((z_i != -m_render_distance) << 1) | ((y_i == 1) << 2);
                        // Generate values for all points
                        m_density_generator->bind();
                        m_density_generator->setUniformInt("u_points_per_axis", m_points_per_axis);
                        m_density_generator->setUniformInt("u_resolution", m_resolution);
                        m_density_generator->setUniformVector3f("u_position_offset", static_cast<glm::vec3>(chunk_coordinate));
                        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, chunk->getDensityDistributionBuffer());
                        m_density_generator->dispatchCompute(m_resolution, m_resolution, m_resolution);
                        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

                        generateMesh(chunk, chunk_coordinate, has_neighbors);
                        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
                    }
                }
            }
        }
        // Setup colliders
        r_game_system.getGpuSynchronizer().setBarrier([this]
        {
            std::vector<int unsigned> mesh_info(6);
            std::vector<float> mesh(m_max_triangle_count * 18);
            for (auto & chunk : m_chunk_pool)
            {
                glGetNamedBufferSubData(chunk.getDrawIndirectBuffer(), 0, sizeof(int unsigned) * 6, mesh_info.data());
                chunk.setMeshInfo(mesh_info[0]);
                if (std::abs(chunk.getPosition().x - m_last_chunk_coords.x) > 1 || std::abs(chunk.getPosition().z - m_last_chunk_coords.z) > 1) continue;
                glGetNamedBufferSubData(chunk.getMeshVB(), 0, m_max_triangle_count * sizeof(float) * 18, mesh.data());
                chunk.setMeshCollider(mesh, m_chunk_collider_material, m_chunk_size_in_units);
            }
        });
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

    void World::update(float delta_time, Window const & window, FirstPersonCamera & camera)
    {
        m_player.update(delta_time, window, camera);
        camera.setPosition(m_player.getPosition());
        onPlayerMoved(m_player.getPosition());
        if (glfwGetMouseButton(window.getWindowHandle(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS || glfwGetMouseButton(window.getWindowHandle(), GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
        {
            castRay(camera); // This is literally still 10x faster than PhysX raycasts
        }
        m_scene->simulate(delta_time);
        m_scene->fetchResults(true);
    }

    void World::render(FirstPersonCamera const & camera)
    {
        m_chunk_renderer->bind();
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_generation_config_u);
        m_chunk_renderer->setUniformMatrix4f("u_view", camera.getViewMatrix());
        m_chunk_renderer->setUniformMatrix4f("u_projection", camera.getProjectionMatrix());
        m_chunk_renderer->setUniformVector3f("u_camera_position_W", camera.getPosition());
        glBindVertexArray(m_chunk_va);
        for (auto & chunk : m_chunk_pool)
        {
            if (!chunk.isActive()) continue;
            m_chunk_renderer->setUniformFloat("u_points_per_axis", static_cast<float>(m_points_per_axis));
            m_chunk_renderer->setUniformMatrix4f("u_model", glm::scale(glm::mat4(1.0f), glm::vec3(m_chunk_size_in_units)) * glm::translate(glm::mat4(1.0f), static_cast<glm::vec3>(chunk.getPosition())));
            VertexArray::bindVertexBuffer(m_chunk_va, chunk.getMeshVB(), VertexDataLayout::POSITION_NORMAL_3F);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, chunk.getDrawIndirectBuffer());
            glDrawArraysIndirect(GL_TRIANGLES, nullptr);
        }
    }
    
    void World::refreshGenerationSpec()
    {
        m_generation_spec = m_density_generator->getBlockUniformInfo();
        for (auto const & block_variable : m_generation_spec)
        {
            ENG_LOG_F("%s, %d, %d", block_variable.m_name.c_str(), block_variable.m_type, block_variable.m_buffer_offset);
        }
    }

    void World::updateGenerationConfig(float const * buffer_data)
    {
        glNamedBufferSubData(m_generation_config_u, 0, m_generation_spec.size() * sizeof(float), buffer_data);
    }

    void World::setRenderDistance(int unsigned render_distance)
    {
        m_render_distance = render_distance;
    }

    void World::setPointsPerAxis(int unsigned exponent)
    {
        if (exponent < 1) exponent = 1;
        if (exponent > 5) exponent = 5;
        m_points_per_axis = static_cast<int>(std::pow(2, exponent));
        m_resolution = static_cast<int>(std::ceil(static_cast<float>(m_points_per_axis) / WORK_GROUP_SIZE));
        m_max_triangle_count = (m_points_per_axis - 1) * (m_points_per_axis - 1) * (m_points_per_axis - 1) * 5;
        initDynamicBuffers();
    }

    int unsigned World::getPointsPerAxis()
    {
        return m_points_per_axis;
    }

    std::vector<Shader::BlockVariable> const & World::getGenerationSpec() const
    {
        return m_generation_spec;
    }
}