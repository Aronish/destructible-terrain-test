#pragma once

#include <glm/glm.hpp>

namespace eng
{
	void qef_add(glm::vec3 n, glm::vec3 p, glm::mat3 & ATA, glm::vec3 & ATb, glm::vec4 & pointaccum);
	float qef_solve(glm::mat3 ATA, glm::vec3 ATb, glm::vec4 pointaccum, glm::vec3 & x);
}