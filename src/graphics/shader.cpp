#include <vector>
#include <unordered_map>

#include "logger.hpp"

#include "graphics/shader.hpp"

namespace eng
{
    static GLenum customShaderTypeToGLenum(std::string const & shader_type_token)
    {
        if (shader_type_token == "vert") return GL_VERTEX_SHADER;
        if (shader_type_token == "frag") return GL_FRAGMENT_SHADER;
        if (shader_type_token == "comp") return GL_COMPUTE_SHADER;
        ENG_LOG_F("Unknown or unsupported shader type %s", shader_type_token.c_str());
        return 0;
    }

    static std::unordered_map<GLenum, std::string> parseCustomShader(char const * file_path)
    {
        std::ifstream file(file_path, std::ios::in);
        std::string source{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        // Break shaders according to #shader <shader_type>
        std::unordered_map<GLenum, std::string> shader_sources;

        const char * typeToken = "#shader";
		size_t shader_type_token_length = std::strlen(typeToken);
		size_t pos = source.find(typeToken, 0);                                     //Start of line with #shader <shader_type>
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);                         //End of shader type directive line
			size_t type_begin = pos + shader_type_token_length + 1;                 //Start of shader type after #shader
			std::string shader_type = source.substr(type_begin, eol - type_begin);

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);             //Start of shader code after #shader directive line
			pos = source.find(typeToken, nextLinePos);                              //Start of next #shader directive line

			shader_sources[customShaderTypeToGLenum(shader_type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}
        return shader_sources;
    }

    static std::vector<GLuint> compileCustomShaders(std::unordered_map<GLenum, std::string> const & shader_sources)
    {
        std::vector<GLuint> compiled_shaders;
        for (auto & map_pair : shader_sources)
        {
            GLuint shader_handle = glCreateShader(map_pair.first);
            const GLchar * src = map_pair.second.c_str();
            glShaderSource(shader_handle, 1, &src, nullptr);
            glCompileShader(shader_handle);

#ifdef ENG_DEBUG
            GLint is_compiled;
            glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &is_compiled);
            if (is_compiled == GL_FALSE)
            {
                ENG_LOG_F("Failed to compile type %d shader!", map_pair.first);

                constexpr GLsizei MAX_LOG_LENGTH = 512;
                GLsizei log_length = 0;
                GLchar info_log[MAX_LOG_LENGTH];
                glGetShaderInfoLog(shader_handle, MAX_LOG_LENGTH, &log_length, &info_log[0]);

                glDeleteShader(shader_handle);

                ENG_LOG(info_log);
            }
#endif
            compiled_shaders.push_back(shader_handle);
        }
        return compiled_shaders;
    }

    Shader::Shader(char const * file_path)
    {
        compile(file_path);
    }

    void Shader::compile(char const * file_path)
    {
        std::vector<GLuint> compiled_shaders = compileCustomShaders(parseCustomShader(file_path));

        m_id = glCreateProgram();

        for (auto & shader_handle : compiled_shaders)
        {
            glAttachShader(m_id, shader_handle);
        }

        glLinkProgram(m_id);
        glValidateProgram(m_id);

#ifdef ENG_DEBUG
        GLint is_linked = 0;
        glGetProgramiv(m_id, GL_LINK_STATUS, &is_linked);
        if (is_linked == GL_FALSE)
        {
            ENG_LOG("Failed to link program!");

            constexpr GLsizei MAX_LOG_LENGTH = 512;
            GLsizei log_length = 0;
            GLchar info_log[MAX_LOG_LENGTH];
            glGetProgramInfoLog(m_id, MAX_LOG_LENGTH, &log_length, &info_log[0]);

            glDeleteProgram(m_id);

            for (auto & shader_handle : compiled_shaders)
            {
                glDeleteShader(shader_handle);
            }

            ENG_LOG_F("Log length: %d", log_length);
            ENG_LOG(info_log);
            return;
        }
#endif

        for (auto & shader_handle : compiled_shaders)
        {
            glDeleteShader(shader_handle);
        }

        // Cache all uniform locations
        m_uniform_locations.clear();
        GLint count;
        glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &count);

        GLchar uniform_name[256];
        GLsizei length;
        GLint size;
        GLenum type;
        for (GLint i = 0; i < count; ++i)
        {
            glGetActiveUniform(m_id, i, sizeof(uniform_name), &length, &size, &type, uniform_name);
            GLint uniform_location = glGetUniformLocation(m_id, uniform_name);
            m_uniform_locations[uniform_name] = uniform_location;
        }
    }

    Shader::~Shader()
    {
        glDeleteProgram(m_id);
    }

    void Shader::bind() const
    {
        glUseProgram(m_id);
    }

    void Shader::dispatchCompute(GLint num_groups_x, GLint num_groups_y, GLint num_groups_z) const
    {
        glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
    }

    void Shader::setUniformMatrix3f(char const * name, glm::mat3 const & data)
    {
#ifdef ENG_DEBUG
        if (m_uniform_locations.find(name) == m_uniform_locations.end())
        {
            ENG_LOG_F("Matrix4fv uniform with name %s does not exist!", name);
            return;
        }
#endif
        GLuint location = m_uniform_locations[name];
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(data));
    }

    void Shader::setUniformMatrix4f(char const * name, glm::mat4 const & data)
    {
#ifdef ENG_DEBUG
        if (m_uniform_locations.find(name) == m_uniform_locations.end())
        {
            ENG_LOG_F("Matrix4fv uniform with name %s does not exist!", name);
            return;
        }
#endif
        GLuint location = m_uniform_locations[name];
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data));
    }

    void Shader::setUniformVector3f(char const * name, glm::vec3 const & data)
    {
#if ENG_DEBUG
        if (m_uniform_locations.find(name) == m_uniform_locations.end())
        {
            ENG_LOG_F("Vector3f uniform with name %s does not exist!", name);
            return;
        }
#endif
        GLuint location = m_uniform_locations[name];
        glUniform3f(location, data.x, data.y, data.z);
    }

    void Shader::setUniformFloat(char const * name, float data)
    {
#ifdef ENG_DEBUG
        if (m_uniform_locations.find(name) == m_uniform_locations.end())
        {
            ENG_LOG_F("Float uniform with name %s does not exist!", name);
            return;
        }
#endif
        GLuint location = m_uniform_locations[name];
        glUniform1f(location, data);
    }

    void Shader::setUniformInt(char const * name, int data)
    {
#ifdef ENG_DEBUG
        if (m_uniform_locations.find(name) == m_uniform_locations.end())
        {
            ENG_LOG_F("Float uniform with name %s does not exist!", name);
            return;
        }
#endif
        GLuint location = m_uniform_locations[name];
        glUniform1i(location, data);
    }
}