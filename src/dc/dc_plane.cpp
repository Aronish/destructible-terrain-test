#include <algorithm>
#include <array>

#include "dc/qef.hpp"
#include "graphics/vertex_array.hpp"

#include "dc_plane.hpp"

namespace eng
{
    DCPlane::DCPlane(GameSystem & game_system)
	{
        m_quad_va = game_system.getAssetManager().createVertexArray();
        m_quad_data = game_system.getAssetManager().createBuffer();
        m_quad_indices = game_system.getAssetManager().createBuffer();

        m_dual_contouring = game_system.getAssetManager().getShader("res/shaders/dual_contouring.glsl");

        int constexpr WIDTH = 20;

        auto circle = [](float x, float y) -> float
        {
            x = x - WIDTH / 2.0f;
            y = y - WIDTH / 2.0f;
            return x * x + y * y + x * y - WIDTH / 1.2f;
        };
        auto circle_grad = [](float x, float y) -> glm::vec2
        {
            x = x - WIDTH / 2.0f;
            y = y - WIDTH / 2.0f;
            return { 2 * x + y, 2 * y + x };
        };

        // Generate density
        std::array<float, WIDTH * WIDTH> signs{};
        for (int y = 0; y < WIDTH; ++y)
        {
            for (int x = 0; x < WIDTH; ++x)
            {
                signs[x + y * WIDTH] = circle(static_cast<float>(x), static_cast<float>(y));
            }
        }

        std::array<std::pair<bool, glm::vec2>, (WIDTH - 1) * (WIDTH - 1)> cells{};
        // Returns the gradient of the best point on the edge (where mc would have a vertex)
        auto check_edge = [circle_grad](glm::vec3 a, glm::vec3 b, glm::vec2 & out_vertex) -> glm::vec2
        {
            if (a.z < 0.0f != b.z < 0.0f)
            {
                float t = -a.z / (b.z - a.z);
                out_vertex = glm::vec2{ a } + t * (glm::vec2{ b } - glm::vec2{ a });
                return glm::normalize(circle_grad(out_vertex.x, out_vertex.y));
            }
            return {};
        };

        // Calculate one vertex per cell
        for (int x = 0; x < WIDTH - 1; ++x)
        {
            for (int y = 0; y < WIDTH - 1; ++y)
            {
                float tl = signs.at(x + y * WIDTH);
                float tr = signs.at(x + 1 + y * WIDTH);
                float bl = signs.at(x + (y + 1) * WIDTH);
                float br = signs.at(x + 1 + (y + 1) * WIDTH);
                glm::vec2 vertices[4];
                glm::vec2 normals[4] =
                {
                    check_edge(glm::vec3{ x,        y,      tl }, glm::vec3{ x + 1, y,      tr }, vertices[0]),
                    check_edge(glm::vec3{ x + 1,    y,      tr }, glm::vec3{ x + 1, y + 1,  br }, vertices[1]),
                    check_edge(glm::vec3{ x + 1,    y + 1,  br }, glm::vec3{ x,     y + 1,  bl }, vertices[2]),
                    check_edge(glm::vec3{ x,        y + 1,  bl }, glm::vec3{ x,     y,      tl }, vertices[3])
                };

                glm::vec4 pointaccum = glm::vec4(0.0);
                glm::mat3 ATA = glm::mat3(0.0);
                glm::vec3 ATb = glm::vec3(0.0);

                int non_zero_normals = 0;
                for (int i = 0; i < 4; ++i)
                {
                    if (normals[i] != glm::vec2{})
                    {
                        qef_add(glm::vec3(normals[i], 0.0f), glm::vec3(vertices[i], 0.0f), ATA, ATb, pointaccum);
                        ++non_zero_normals;
                    }
                }
                if (non_zero_normals != 0 || (tl < 0.0f && tr < 0.0f && bl < 0.0f && br < 0.0f))
                {
                    glm::vec3 com = glm::vec3(pointaccum) / pointaccum.w;

                    glm::vec3 x_q;
                    float error = qef_solve(ATA, ATb, pointaccum, x_q);

                    //printf("masspoint = (%.5f %.5f %.5f)\n", com.x, com.y, com.z);
                    //printf("point = (%.5f %.5f %.5f)\n", x_q.x, x_q.y, x_q.z);
                    //printf("error = %.5f\n", error);
                    //ENG_LOG_F("%f, %f", x_q.x, x_q.y);
                    cells[x + y * (WIDTH - 1)] = { true, (non_zero_normals == 0 ? glm::vec2{ static_cast<float>(x), static_cast<float>(y) } : glm::vec2{ x_q.x, x_q.y }) };
                }
            }
        }

        std::vector<float> vertices;
//#define POINTS
#ifndef POINTS
        for (int y = 0; y < WIDTH - 1; ++y)
        {
            for (int x = 0; x < WIDTH - 1; ++x)
            {
                if (x < WIDTH - 2 && y < WIDTH - 2)
                {
                    auto & xy = cells.at(x + y * (WIDTH - 1));
                    auto & x_1y = cells.at(x + 1 + y * (WIDTH - 1));
                    auto & xy_1 = cells.at(x + (y + 1) * (WIDTH - 1));
                    if (xy.first && x_1y.first && xy_1.first)
                    {
                        vertices.push_back(xy.second.x);
                        vertices.push_back(xy.second.y);
                        vertices.push_back(x_1y.second.x);
                        vertices.push_back(x_1y.second.y);
                        vertices.push_back(xy_1.second.x);
                        vertices.push_back(xy_1.second.y);
                        m_vertex_count += 3;
                    }
                }
                if (x > 0 && y > 0)
                {
                    auto & xy = cells.at(x + y * (WIDTH - 1));
                    auto & x_1y = cells.at(x - 1 + y * (WIDTH - 1));
                    auto & xy_1 = cells.at(x + (y - 1) * (WIDTH - 1));
                    if (xy.first && x_1y.first && xy_1.first)
                    {
                        vertices.push_back(xy.second.x);
                        vertices.push_back(xy.second.y);
                        vertices.push_back(x_1y.second.x);
                        vertices.push_back(x_1y.second.y);
                        vertices.push_back(xy_1.second.x);
                        vertices.push_back(xy_1.second.y);
                        m_vertex_count += 3;
                    }
                }
            }
        }
#else
        for (auto [has_vertex, vertex] : cells)
        {
            if (has_vertex)
            {
                vertices.push_back(vertex.x);
                vertices.push_back(vertex.y);
                ++m_vertex_count;
            }
        }
#endif
        //std::vector<int> indices(vertices.size());
        //std::generate(indices.begin(), indices.end(), [i = 0]() mutable -> int { return ++i; });

        glNamedBufferData(m_quad_data, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        //glNamedBufferData(m_quad_indices, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
        VertexArray::associateVertexBuffer(m_quad_va, m_quad_data, VertexDataLayout::FLOAT2);
        //VertexArray::associateIndexBuffer(m_quad_va, m_quad_indices, indices.data(), indices.size() * sizeof(int));
	}

    GLuint DCPlane::getVertexArray() const
    {
        return m_quad_va;
    }

    int DCPlane::getIndexCount() const
    {
        return m_vertex_count;
    }

    bool DCPlane::meshEmpty() const
    {
        return m_vertex_count == 0;
    }
}