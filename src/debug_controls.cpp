#include <imgui.h>

#include "debug_controls.hpp"

namespace eng
{
    void DebugControls::onShaderBlockChanged(int num_variables)
    {
        m_generation_data.clear();
        m_generation_data.resize(num_variables);
    }

    bool DebugControls::render(std::vector<Shader::BlockVariable> const & generation_spec)
	{
        bool values_changed = false;
        /*
        values_changed |= ImGui::DragFloat("Chunk Size", &world.m_chunk_size_in_units, 0.1f, 0.0f, FLT_MAX);
        values_changed |= ImGui::DragFloat("Threshold", &world.m_threshold, 0.05f);
        ImGui::DragFloat("Terraform Radius", &world.m_terraform_radius, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Terraform Strength", &world.m_terraform_strength, 0.01f, -100.0f, 100.0f);

        if (values_changed |= ImGui::Checkbox("Tweakable Lacunarity/Persistence", &m_tweakable_lac_per))
        {
            if (!m_tweakable_lac_per)
            {
                m_config.m_lacunarity_2d = 2.0f;
                m_config.m_persistence_2d = 0.5f;
                m_config.m_lacunarity_3d = 2.0f;
                m_config.m_persistence_3d = 0.5f;
            }
        }

        ImGui::Separator();
        ImGui::Text("2D Noise");
        if (values_changed |= ImGui::InputInt("Octaves 2D", &m_config.m_octaves_2d, 1, 1))
        {
            if (m_config.m_octaves_2d < 1) m_config.m_octaves_2d = 1;
        }
        values_changed |= ImGui::DragFloat("Frequency 2D", &m_config.m_frequency_2d, 0.01f, 0.0f);
        values_changed |= ImGui::DragFloat("Amplitude 2D", &m_config.m_amplitude_2d, 0.01f, 0.0f);
        values_changed |= ImGui::DragFloat("Exponent 2D", &m_config.m_exponent_2d, 0.01f, 0.0f);

        if (m_tweakable_lac_per)
        {
            values_changed |= ImGui::DragFloat("Lacunarity 2D", &m_config.m_lacunarity_2d, 0.01f, 0.0f);
            values_changed |= ImGui::DragFloat("Persistence 2D", &m_config.m_persistence_2d, 0.01f, 0.0f);
        }

        ImGui::Separator();
        ImGui::Text("3D Noise");
        if (values_changed |= ImGui::InputInt("Octaves 3D", &m_config.m_octaves_3d, 1, 1))
        {
            if (m_config.m_octaves_3d < 1) m_config.m_octaves_3d = 1;
        }
        values_changed |= ImGui::DragFloat("Frequency 3D", &m_config.m_frequency_3d, 0.01f, 0.0f);
        values_changed |= ImGui::DragFloat("Amplitude 3D", &m_config.m_amplitude_3d, 0.01f, 0.0f);
        values_changed |= ImGui::DragFloat("Exponent 3D", &m_config.m_exponent_3d, 0.01f, 0.0f);

        if (m_tweakable_lac_per)
        {
            values_changed |= ImGui::DragFloat("Lacunarity 3D", &m_config.m_lacunarity_3d, 0.01f, 0.0f);
            values_changed |= ImGui::DragFloat("Persistence 3D", &m_config.m_persistence_3d, 0.01f, 0.0f);
        }
        */

        for (auto const & block_variable : generation_spec)
        {
            switch (block_variable.m_type)
            {
                case GL_INT:
                {
                    auto & value = m_generation_data[block_variable.m_buffer_offset / sizeof(ArithmeticType)];
                    if (value.i == -1) value = ArithmeticType{ 0 };
                    values_changed |= ImGui::DragInt(block_variable.m_name.c_str(), &value.i, 0.05f);
                    break;
                }
                case GL_FLOAT:
                {
                    auto & value = m_generation_data[block_variable.m_buffer_offset / sizeof(ArithmeticType)];
                    if (value.i == -1)
                    {
                        value = ArithmeticType{ .f = 0.0f };
                    }
                    values_changed |= ImGui::DragFloat(block_variable.m_name.c_str(), &value.f, 0.05f);
                    break;
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