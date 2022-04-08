#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace eng
{
    class Shader
    {
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
        void setUniformVector3f(char const * name, glm::vec3 const & data);
        void setUniformFloat(char const * name, float data);
        void setUniformInt(char const * name, int data);
    };
}