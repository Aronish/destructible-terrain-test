#include <algorithm>
#include <array>
#include <chrono>

#include "dc/qef.hpp"
#include "graphics/vertex_array.hpp"

#include "dc_plane.hpp"

namespace eng
{
    DCPlane::DCPlane(GameSystem & game_system)
	{
        m_edge_data_generator = game_system.getAssetManager().getShader("res/shaders/generate_edge_data.glsl");
        m_feature_point_generator = game_system.getAssetManager().getShader("res/shaders/generate_feature_points.glsl");
        m_mesher = game_system.getAssetManager().getShader("res/shaders/dc_mesher.glsl");

        m_edge_data = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_edge_data, 3 * (chunk_size + 1) * (chunk_size + 1) * chunk_size * 4 * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

        m_dc_vertices = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_dc_vertices, chunk_size * chunk_size * chunk_size * sizeof(float) * 3, nullptr, GL_DYNAMIC_STORAGE_BIT);

        m_materials = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_materials, WORK_GROUP_SIZE * WORK_GROUP_SIZE * WORK_GROUP_SIZE * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

        m_di_mesh = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_di_mesh, sizeof(s_di_config), s_di_config, GL_DYNAMIC_STORAGE_BIT);

        m_mesh = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_mesh, 3 * (chunk_size + 1) * (chunk_size + 1) * chunk_size * 2 * 3 * 18 * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

        m_points = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_points, chunk_size * chunk_size * chunk_size * 3 * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

        m_normals = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_normals, WORK_GROUP_SIZE * WORK_GROUP_SIZE * WORK_GROUP_SIZE * 6 * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);

        m_di_normals = game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_di_normals, sizeof(s_di_config), s_di_config, GL_DYNAMIC_STORAGE_BIT);

        m_va_mesh = game_system.getAssetManager().createVertexArray();
        VertexArray::associateVertexBuffer(m_va_mesh, m_mesh, VertexDataLayout::FLOAT3_FLOAT3);

        m_va_normals = game_system.getAssetManager().createVertexArray();
        VertexArray::associateVertexBuffer(m_va_normals, m_normals, VertexDataLayout::FLOAT3_FLOAT3);

        generate();
	}

    void DCPlane::generate(bool recompile)
    {
        if (recompile)
        {
            m_edge_data_generator->compile("res/shaders/generate_edge_data.glsl");
            m_feature_point_generator->compile("res/shaders/generate_feature_points.glsl");
            m_mesher->compile("res/shaders/dc_mesher.glsl");
        }
        auto start = std::chrono::high_resolution_clock::now();
        glClearNamedBufferData(m_edge_data, GL_R32F, GL_RED, GL_FLOAT, nullptr);
        glClearNamedBufferData(m_normals, GL_R32F, GL_RED, GL_FLOAT, nullptr);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_edge_data);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_normals);
        glNamedBufferSubData(m_di_normals, 0, sizeof(s_di_config), s_di_config);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_di_normals);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_materials);
        m_edge_data_generator->bind();
        m_edge_data_generator->setUniformFloat("u_threshold", m_threshold);
        m_edge_data_generator->setUniformFloat("u_frequency", m_frequency);
        glDispatchCompute(1, 1, 1);

        glClearNamedBufferData(m_dc_vertices, GL_R32F, GL_RED, GL_FLOAT, nullptr);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_dc_vertices);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_di_mesh);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_points);
        glNamedBufferSubData(m_di_mesh, 0, sizeof(s_di_config), s_di_config);
        m_feature_point_generator->bind();
        glDispatchCompute(1, 1, 1);
        
        glClearNamedBufferData(m_mesh, GL_R32F, GL_RED, GL_FLOAT, nullptr);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_mesh);
        m_mesher->bind();
        glDispatchCompute(1, 1, 1);

        auto end = std::chrono::high_resolution_clock::now();
        ENG_LOG_F("Took: %lld ns", (end - start).count());
    }
}