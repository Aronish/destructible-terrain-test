#pragma once

#include <map>

#include "world/world.hpp"

namespace eng
{
	class DebugControls
	{
	private:
		union ArithmeticType
		{
			int i{ -1 };
			float f;
		};
		std::vector<ArithmeticType> m_generation_data;
		bool m_tweakable_lac_per{};
	public:
		void onShaderBlockChanged(int num_variables);
		bool render(std::vector<Shader::BlockVariable> const & generation_spec);
		float const * getBufferData() const;
	};
}