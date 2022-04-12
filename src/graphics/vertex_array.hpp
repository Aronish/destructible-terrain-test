#pragma once

#include <memory>

#include <glad/glad.h>

#include "shader_storage_buffer.hpp"
#include "vertex_buffer_layout.hpp"

namespace eng::VertexArray
{
    void associateVertexBuffer(GLuint vertex_array, GLuint vertex_buffer, VertexDataLayout const & layout);
    void associateIndexBuffer(GLuint vertex_array, GLuint index_buffer, int * indices, size_t indices_size);
}