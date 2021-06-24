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

        void bind() const;

        void setUniformMatrix4f(char const * name, glm::mat4 const & data);
        void setUniformVector3f(char const * name, glm::vec3 const & data);
        void setUniformFloat(char const * name, float data);
    };
}