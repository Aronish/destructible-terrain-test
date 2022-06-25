#include "player.hpp"

namespace eng
{
	Player::Player(GameSystem & game_system, glm::vec3 const & initial_position) : m_position(initial_position)
	{
		using namespace physx;
		m_rigid_body = game_system.getPhysx()->createRigidDynamic(PxTransform(initial_position.x, initial_position.y, initial_position.z));
		PxTransform relative_pose(PxQuat(PxHalfPi, PxVec3(0, 0, 1)));
		physx::PxMaterial * material = game_system.getPhysx()->createMaterial(0.5f, 0.5f, 0.1f);
		physx::PxShape * capsule_shape = PxRigidActorExt::createExclusiveShape(*m_rigid_body, PxSphereGeometry(0.5f), *material);
		capsule_shape->setLocalPose(relative_pose);
		//capsule_shape->setContactOffset(physx::PxTolerancesScale().length * 1.0f);
		PxRigidBodyExt::updateMassAndInertia(*m_rigid_body, 1.0f);
	}

	void Player::update(float delta_time, Window const & window, FirstPersonCamera const & camera)
	{
		m_rigid_body->clearForce();
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_W))
        {
			m_rigid_body->addForce(physx::PxVec3{ camera.getDirection().x, camera.getDirection().y, camera.getDirection().z } * MOVEMENT_SPEED);
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_S))
        {
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_D))
        {
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_A))
        {
        }
        if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_SPACE))
        {
			m_position = { 0.0f, 15.0f, 0.0f };
			m_rigid_body->setLinearVelocity({ 0.0f, 0.0f, 0.0f });
			m_rigid_body->setGlobalPose(physx::PxTransform(m_position.x, m_position.y, m_position.z));
        }
		auto const & linear_velocity = m_rigid_body->getLinearVelocity();
		m_position += glm::vec3(linear_velocity.x, linear_velocity.y, linear_velocity.z) * delta_time;
	}

	glm::vec3 const & Player::getPosition() const
	{
		return m_position;
	}

	physx::PxRigidDynamic * Player::getDynamicRigidBody() const
	{
		return m_rigid_body;
	}
}