#pragma once

#include <memory>

#include "dc_plane.hpp"
#include "event/event.hpp"
#include "first_person_camera.hpp"
#include "game_system.hpp"
#include "window.hpp"

namespace eng
{
	class DCApplication
	{
	private:
		Window m_window;
		GameSystem m_game_system;
		FirstPersonCamera m_camera;

		DCPlane m_plane;

		std::shared_ptr<Shader> m_quad_shader;
	public:
		DCApplication();

		void run();

		void onEvent(Event const & event);
		void update(float delta_time);
		void render();
	};
}