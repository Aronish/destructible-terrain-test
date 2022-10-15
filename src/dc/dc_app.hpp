#pragma once

#include <memory>

#include <glad/glad.h>

#include "event/event.hpp"
#include "first_person_camera.hpp"
#include "game_system.hpp"
#include "graphics/shader.hpp"
#include "mesh.hpp"
#include "octree.hpp"
#include "window.hpp"

namespace eng
{
	class DcApp
	{
	private:
		float static constexpr WALKING_SPEED = 2.5f, RUN_MULTIPLIER = 1.6f;
		int static constexpr OCTREE_SIZE = 64;
	private:
		Window window_; // Has to be first due to OpenGL initialization
		GameSystem game_system_;
		FirstPersonCamera camera_;

		std::shared_ptr<Shader> shader_;
		std::shared_ptr<Texture> grass_;
		std::shared_ptr<Texture> dirt_;

		Mesh mesh_;
		OctreeNode * root_{};

	public:
		DcApp(int unsigned width, int unsigned height, char const * title, bool maximized);
		~DcApp();

		void run();
		void onEvent(Event const & event);
		void update(float delta_time);
		void render() const;
	};
}
