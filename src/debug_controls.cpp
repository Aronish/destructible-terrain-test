#include <imgui.h>

#include "debug_controls.hpp"

namespace eng
{
    void DebugControls::onShaderBlockChanged(size_t num_variables)
    {
        m_generation_data.clear();
        m_generation_data.resize(num_variables);
    }

    void DebugControls::loadDefaultValues(std::vector<Shader::BlockVariable> const & spec)
    {
        std::ifstream stream(DEFAULTS_FILE, std::ios::in);
        std::string source{ std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
        std::stringstream defaults(source);
        std::string line, name, value;
        while (std::getline(defaults, line, '\n'))
        {
            size_t colon_pos{ line.find(":", 0) };
            name = line.substr(0, colon_pos);
            value = line.substr(colon_pos + 1, line.size() - colon_pos);
            if (auto result = std::find_if(spec.begin(), spec.end(), [&name](Shader::BlockVariable variable) { return variable.m_name == name; }); result != spec.end())
            {
                m_generation_data[(*result).m_buffer_offset / sizeof(IntOrFloat)] = (*result).m_type == GL_INT ? IntOrFloat{ std::stoi(value) } : IntOrFloat{ .f = std::stof(value) };
            }
        }
    }

    void DebugControls::saveDefaultValues(std::vector<Shader::BlockVariable> const & spec)
    {
        std::ofstream stream(DEFAULTS_FILE, std::ios::out);
        for (auto const & variable : spec)
        {
            IntOrFloat value = m_generation_data[variable.m_buffer_offset / sizeof(IntOrFloat)];
            stream << variable.m_name.c_str() << ":" << (variable.m_type == GL_INT ? value.i : value.f) << "\n";
        }
    }

    bool DebugControls::render(World & world)
	{
        bool values_changed = false;

        if (ImGui::CollapsingHeader("World Generation Config"))
        {
            if (ImGui::Button("Load Defaults"))
            {
                loadDefaultValues(world.getGenerationSpec());
                values_changed = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Save As Default")) saveDefaultValues(world.getGenerationSpec());
            values_changed |= ImGui::DragFloat("Threshold", &world.m_threshold, 0.05f);
            for (auto const & block_variable : world.getGenerationSpec())
            {
                switch (block_variable.m_type)
                {
                    case GL_INT:
                    {
                        auto & value = m_generation_data[block_variable.m_buffer_offset / sizeof(IntOrFloat)];
                        if (value.i == -1) value = IntOrFloat{ 0 };
                        values_changed |= ImGui::DragInt(block_variable.m_name.c_str(), &value.i, 0.05f);
                        break;
                    }
                    case GL_FLOAT:
                    {
                        auto & value = m_generation_data[block_variable.m_buffer_offset / sizeof(IntOrFloat)];
                        if (value.i == -1) value = IntOrFloat{ .f = 0.0f };
                        values_changed |= ImGui::DragFloat(block_variable.m_name.c_str(), &value.f, 0.005f);
                        break;
                    }
                }
            }
        }
        return values_changed;
	}

    float const * DebugControls::getBufferData() const
    {
        return &m_generation_data.data()->f;
    }
}