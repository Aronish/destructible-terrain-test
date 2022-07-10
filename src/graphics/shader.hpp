#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

#define ENG_DETECT_INACTIVE_UNIFORMS 0

namespace eng
{
    class Shader
    {
    public:
        struct BlockVariable
        {
            std::string m_name;
            GLenum m_type;
            GLint m_buffer_offset;
        };
    private:
        GLuint m_id;
        std::unordered_map<std::string, GLuint> m_uniform_locations;
    public:
        explicit Shader(char const * file_path);
        ~Shader();

        void compile(char const * file_path);

        void bind() const;
        void dispatchCompute(GLint num_groups_x, GLint num_groups_y, GLint num_groups_z) const;

        void setUniformMatrix3f(char const * name, glm::mat3 const & data);
        void setUniformMatrix4f(char const * name, glm::mat4 const & data);
        void setUniformVector2f(char const * name, glm::vec2 const & data);
        void setUniformVector3f(char const * name, glm::vec3 const & data);
        void setUniformFloat(char const * name, float data);
        void setUniformInt(char const * name, int data);

        std::vector<BlockVariable> getBlockUniformInfo();
    };
}