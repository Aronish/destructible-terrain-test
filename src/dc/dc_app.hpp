#pragma once

#include <memory>

#include <glad/glad.h>

#include "cube_frame.hpp"
#include "event/event.hpp"
#include "first_person_camera.hpp"
#include "game_system.hpp"
#include "graphics/shader.hpp"
#include "octree.hpp"
#include "window.hpp"

namespace eng
{
	class DCApp
	{
    private:
        float static constexpr WALKING_SPEED = 2.5f, RUN_MULTIPLIER = 1.6f;
    private:
        Window m_window; // Has to be first due to OpenGL initialization
        GameSystem m_game_system;
        FirstPersonCamera m_camera;

        std::shared_ptr<Shader> m_shader;
        CubeFrame m_cube;
        Octree m_root;

    public:
        DCApp(int unsigned width, int unsigned height, char const * title, bool maximized);

        void run();
        void onEvent(Event const & event);
        void update(float delta_time);
        void render();
	};
}