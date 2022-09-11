#pragma once

#include "world/world.hpp"

namespace eng
{
	class DebugControls
	{
	private:
		char const constexpr static
			* DEFAULTS_WORLDGEN_FILE = "C:/dev/engineering_game/res/default_world_gen_config.txt",
			* FLAT_PLANE_FILE = "C:/dev/engineering_game/res/flat_plane_config.txt";
	private:
		union IntOrFloat
		{
			int i{ -1 };
			float f;
		};
		std::vector<IntOrFloat> m_generation_data;
		bool m_tweakable_lac_per{};
	public:
		void onShaderBlockChanged(size_t num_variables);
		void loadValuesFromFile(char const * path, std::vector<Shader::BlockVariable> const & spec);
		void saveDefaultValues(std::vector<Shader::BlockVariable> const & spec);
		bool render(World & world);
		float const * getBufferData() const;
	};
}