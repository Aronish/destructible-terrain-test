#include <vector>
#include <unordered_map>

#include "logger.hpp"

#include "graphics/shader.hpp"

namespace eng
{
    static GLenum customShaderTypeToGLenum(std::string const & shader_type_token)
    {
        if (shader_type_token == "vert")        return GL_VERTEX_SHADER;
        if (shader_type_token == "frag")        return GL_FRAGMENT_SHADER;
        if (shader_type_token == "comp")        return GL_COMPUTE_SHADER;
        ENG_LOG_F("Unknown or unsupported shader type %s", shader_type_token.c_str());
        return 0;
    }

    static std::unordered_map<GLenum, std::string> parseCustomShader(char const * file_path)
    {
        std::ifstream file(file_path, std::ios::in);
        std::string source{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        // Break shaders according to #shader <shader_type>
        std::unordered_map<GLenum, std::string> shader_sources;

        char const * type_token = "#shader";
        size_t shader_type_token_length = std::strlen(type_token);
        size_t pos = source.find(type_token, 0);                                    //Start of line with #shader <shader_type>
        while (pos != std::string::npos)
        {
            size_t eol = source.find_first_of("\r\n", pos);                         //End of shader type directive line
            size_t type_begin = pos + shader_type_token_length + 1;                 //Start of shader type after #shader
            std::string shader_type = source.substr(type_begin, eol - type_begin);

            size_t next_line_pos = source.find_first_not_of("\r\n", eol);           //Start of shader code after #shader directive line
            pos = source.find(type_token, next_line_pos);                           //Start of next #shader directive line

            shader_sources[customShaderTypeToGLenum(shader_type)] = (pos == std::string::npos) ? source.substr(next_line_pos) : source.substr(next_line_pos, pos - next_line_pos);
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
        ENG_LOG_F("Compiled shader %s", file_path);
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
        GLint uniform_count;
        glGetProgramInterfaceiv(m_id, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniform_count);
        GLenum const properties[] = { GL_BLOCK_INDEX, GL_NAME_LENGTH, GL_LOCATION };
        size_t constexpr num_properties = std::size(properties);
        for (int uniform = 0; uniform < uniform_count; ++uniform)
        {
            GLint values[num_properties];
            glGetProgramResourceiv(m_id, GL_UNIFORM, uniform, num_properties, properties, num_properties, nullptr, values);
            if (values[0] != -1) continue; // Skips uniforms in blocks

            std::string name(values[1], ' ');
            glGetProgramResourceName(m_id, GL_UNIFORM, uniform, static_cast<GLsizei>(values[1]), nullptr, name.data());
            name.pop_back(); // \0 means nothing in an std::string, but this mf ^ will add one regardless
            m_uniform_locations[name] = values[2];
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

#if defined(ENG_DEBUG) && ENG_CHECK_UNIFORMS
    #if ENG_CHECK_UNIFORMS_VERBOSE
        #define ENG_UNIFORM_CHECKER if (m_uniform_locations.find(name) == m_uniform_locations.end()) { ENG_LOG_F("Matrix4fv uniform with name %s does not exist!", name); return; }
    #else
        #define ENG_UNIFORM_CHECKER if (m_uniform_locations.find(name) == m_uniform_locations.end()) { return; }
    #endif
#else
    #define ENG_UNIFORM_CHECKER
#endif

    void Shader::setUniformMatrix3f(char const * name, glm::mat3 const & data)
    {
        ENG_UNIFORM_CHECKER;
        GLuint location = m_uniform_locations.at(name);
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(data));
    }

    void Shader::setUniformMatrix4f(char const * name, glm::mat4 const & data)
    {
        ENG_UNIFORM_CHECKER;
        GLuint location = m_uniform_locations.at(name);
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data));
    }

    void Shader::setUniformVector2f(char const * name, glm::vec2 const & data)
    {
        ENG_UNIFORM_CHECKER;
        GLuint location = m_uniform_locations.at(name);
        glUniform2f(location, data.x, data.y);
    }

    void Shader::setUniformVector3f(char const * name, glm::vec3 const & data)
    {
        ENG_UNIFORM_CHECKER;
        GLuint location = m_uniform_locations.at(name);
        glUniform3f(location, data.x, data.y, data.z);
    }

    void Shader::setUniformVector4f(char const * name, glm::vec4 const & data)
    {
        ENG_UNIFORM_CHECKER;
        GLuint location = m_uniform_locations.at(name);
        glUniform4f(location, data.x, data.y, data.z, data.w);
    }

    void Shader::setUniformFloat(char const * name, float data)
    {
        ENG_UNIFORM_CHECKER;
        GLuint location = m_uniform_locations.at(name);
        glUniform1f(location, data);
    }

    void Shader::setUniformInt(char const * name, int data)
    {
        ENG_UNIFORM_CHECKER;
        GLuint location = m_uniform_locations.at(name);
        glUniform1i(location, data);
    }

    void Shader::setUniformUInt(char const * name, int unsigned data)
    {
        ENG_UNIFORM_CHECKER;
        GLuint location = m_uniform_locations.at(name);
        glUniform1ui(location, data);
    }

    std::vector<Shader::BlockVariable> Shader::getBlockUniformInfo()
    {
        std::vector<BlockVariable> block_uniform_names;
        GLint uniform_count;
        glGetProgramInterfaceiv(m_id, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniform_count);
        GLenum const properties[] = { GL_BLOCK_INDEX, GL_NAME_LENGTH, GL_TYPE, GL_OFFSET };
        size_t constexpr num_properties = std::size(properties);
        for (int uniform = 0; uniform < uniform_count; ++uniform)
        {
            GLint values[num_properties];
            glGetProgramResourceiv(m_id, GL_UNIFORM, uniform, num_properties, properties, num_properties, nullptr, values);
            if (values[0] == -1) continue; // Skips bare uniforms

            std::string name(values[1], ' ');
            glGetProgramResourceName(m_id, GL_UNIFORM, uniform, static_cast<GLsizei>(values[1]), nullptr, name.data());
            name.pop_back(); // \0 means nothing in an std::string, but this mf ^ will add one regardless
            block_uniform_names.emplace_back(name, values[2], values[3]);
        }
        return block_uniform_names;
    }
}