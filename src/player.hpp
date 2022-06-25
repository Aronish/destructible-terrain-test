#pragma once

#include <PxPhysicsAPI.h>

#include <glm/glm.hpp>

#include "first_person_camera.hpp"
#include "game_system.hpp"
#include "window.hpp"

namespace eng
{
	class Player
	{
	private:
		float inline static constexpr MOVEMENT_SPEED = 2.5f;

	private:
		glm::vec3 m_position{};
		physx::PxRigidDynamic * m_rigid_body;

	public:
		Player(GameSystem & game_system, glm::vec3 const & initial_position = {});

		void update(float delta_time, Window const & window, FirstPersonCamera const & camera);

		glm::vec3 const & getPosition() const;

		physx::PxRigidDynamic * getDynamicRigidBody() const;
	};
}