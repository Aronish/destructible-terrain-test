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
		float static constexpr WALKING_SPEED = 2.5f, RUN_MULTIPLIER = 1.6f;

	private:
		physx::PxController * m_character_controller;
		physx::PxVec3 m_velocity{ physx::PxZERO::PxZero };
		physx::PxControllerCollisionFlags m_collision_flags;
		bool m_on_ground{};

	public:
		void initCharacterController(physx::PxControllerManager * controller_manager, GameSystem & game_system, physx::PxExtendedVec3 const & initial_position);

		void update(float delta_time, Window const & window, FirstPersonCamera const & camera, bool flight);

		glm::vec3 getPosition() const;
	};
}