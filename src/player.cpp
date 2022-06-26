#include "player.hpp"

namespace eng
{
	void Player::initCharacterController(physx::PxControllerManager * controller_manager, GameSystem & game_system, physx::PxExtendedVec3 const & initial_position)
	{
		physx::PxCapsuleControllerDesc capsule_desc;
		capsule_desc.height = 1.0f;
		capsule_desc.radius = 0.25f;
		capsule_desc.position = initial_position;
		capsule_desc.material = game_system.getPhysx()->createMaterial(1.0f, 1.0f, 0.1f);
		m_character_controller = controller_manager->createController(capsule_desc);
	}

	void Player::update(float delta_time, Window const & window, FirstPersonCamera const & camera)
	{
		m_velocity.x = m_velocity.z = 0.0f;

		if (m_on_ground && glfwGetKey(window.getWindowHandle(), GLFW_KEY_SPACE))
		{
			m_on_ground = false;
			m_velocity.y += 4.0f;
		}

		m_velocity.y -= 9.81f * delta_time;

		float speed = WALKING_SPEED * (glfwGetKey(window.getWindowHandle(), GLFW_KEY_LEFT_SHIFT) ? RUN_MULTIPLIER : 1.0f);
		glm::vec2 player_direction = glm::vec2{ camera.getDirection().x, camera.getDirection().z } * speed;

        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_W))
        {
			m_velocity += physx::PxVec3(player_direction.x, 0.0f, player_direction.y);
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_A))
        {
			player_direction = glm::mat2(0.0f, -1.0f, 1.0f, 0.0f) * player_direction;
			m_velocity += physx::PxVec3(player_direction.x, 0.0f, player_direction.y);
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_S))
        {
			player_direction = -player_direction;
			m_velocity += physx::PxVec3(player_direction.x, 0.0f, player_direction.y);
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_D))
        {
			player_direction = glm::mat2(0.0f, 1.0f, -1.0f, 0.0f) * player_direction;
			m_velocity += physx::PxVec3(player_direction.x, 0.0f, player_direction.y);
        }
		if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_Q))
		{
			m_velocity = {};
			m_character_controller->setPosition({ 0.0f, 15.0f, 0.0f });
		}
		m_collision_flags = m_character_controller->move(m_velocity * delta_time, 0.0f, delta_time, physx::PxControllerFilters());
		if (m_collision_flags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
		{
			m_on_ground = true;
			m_velocity.y = 0.0f;
		}
	}
	
	glm::vec3 const & Player::getPosition() const
	{
		auto const & [x, y, z] = m_character_controller->getPosition();
		return glm::vec3(x, y, z);
	}
}