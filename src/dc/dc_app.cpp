#include <chrono>

#include "event/key_event.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/vertex_data_layout.hpp"

#include "dc_app.hpp"

namespace eng
{
	DcApp::DcApp(int unsigned const width, int unsigned const height, char const * title, bool const maximized) :
		window_(width, height, title, maximized, [this]<typename Event>(Event && event)
		{
			onEvent(std::forward<Event>(event));
		}),
		camera_(width, height),
		shader_(game_system_.getAssetManager().getShader("res/shaders/chunk.glsl")),
		grass_(game_system_.getAssetManager().getTexture("res/textures/TexturesCom_Grass0157_1_seamless_S.jpg")),
		dirt_(game_system_.getAssetManager().getTexture("res/textures/TexturesCom_SoilMud0044_1_seamless_S.jpg")),
		mesh_(game_system_)
	{
		glfwSwapInterval(1);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.79f, 0.94f, 1.0f, 1.0f);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Mesh::VertexBuffer vertices;
		std::vector<int> indices;
		
		auto start = std::chrono::high_resolution_clock::now();
		root_ = OctreeNode::buildOctree(glm::ivec3(-OCTREE_SIZE / 2), OCTREE_SIZE, -1.0f);
		OctreeNode::generateMeshFromOctree(root_, vertices, indices);
		mesh_.refreshMesh(vertices, indices);
		auto end = std::chrono::high_resolution_clock::now();
		printf("Took %lld ns", (end - start).count());
	}

	DcApp::~DcApp()
	{
		OctreeNode::destroyOctree(root_);
	}

	void DcApp::onEvent(Event const & event)
	{
		EventDispatcher::dispatch<WindowResizedEvent>(event, &FirstPersonCamera::onWindowResized, &camera_);
		if (!event.m_window.isCursorVisible())
		{
			EventDispatcher::dispatch<MouseMovedEvent>(event, &FirstPersonCamera::onMouseMoved, &camera_);
		}
		EventDispatcher::dispatch<KeyPressedEvent>(event, [&](KeyPressedEvent const & key_pressed_event)
		{
			switch (key_pressed_event.m_key_code)
			{
			case GLFW_KEY_E:
				window_.setCursorVisibility(!window_.isCursorVisible());
				camera_.setCursorPosition(key_pressed_event.m_window.getCursorPosition());
				break;
			case GLFW_KEY_F:
				window_.setFullscreen(!window_.isFullscreen());
				break;
			default:
				break;
			}
		});
	}

	void DcApp::update(float const delta_time)
	{
		glfwPollEvents();
		game_system_.getGpuSynchronizer().update();
		if (!window_.isCursorVisible())
		{
			float const speed = WALKING_SPEED * (glfwGetKey(window_.getWindowHandle(), GLFW_KEY_LEFT_SHIFT)
				                                     ? RUN_MULTIPLIER
				                                     : 1.0f);
			float const cos_yaw = std::cosf(camera_.getYaw()), sin_yaw = std::sinf(camera_.getYaw());
			glm::vec2 direction{};
			float y_velocity{};
			bool moved{};

			if (glfwGetKey(window_.getWindowHandle(), GLFW_KEY_W)) direction += glm::vec2{cos_yaw, sin_yaw};
			if (glfwGetKey(window_.getWindowHandle(), GLFW_KEY_A)) direction += glm::mat2(0.0f, -1.0f, 1.0f, 0.0f) *
				glm::vec2{cos_yaw, sin_yaw};
			if (glfwGetKey(window_.getWindowHandle(), GLFW_KEY_S)) direction += -glm::vec2{cos_yaw, sin_yaw};
			if (glfwGetKey(window_.getWindowHandle(), GLFW_KEY_D)) direction += glm::mat2(0.0f, 1.0f, -1.0f, 0.0f) *
				glm::vec2{cos_yaw, sin_yaw};
			if (glfwGetKey(window_.getWindowHandle(), GLFW_KEY_SPACE)) y_velocity = speed;
			else if (glfwGetKey(window_.getWindowHandle(), GLFW_KEY_C)) y_velocity = -speed;
			if (length(direction) > 0.0f)
			{
				direction = normalize(direction) * speed;
				moved = true;
			}
			if (y_velocity != 0.0f) moved = true;
			if (moved) camera_.addPosition(glm::vec3{direction.x, y_velocity, direction.y} * delta_time);
		}
	}

	void DcApp::render() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader_->bind();
		shader_->setUniformMatrix4f("u_model", glm::scale(glm::mat4(1.0f), glm::vec3(0.1f)));
		shader_->setUniformMatrix4f("u_view", camera_.getViewMatrix());
		shader_->setUniformMatrix4f("u_projection", camera_.getProjectionMatrix());
		shader_->setUniformVector3f("u_camera_position_W", camera_.getPosition());
		grass_->bind(0);
		dirt_->bind(1);
		glBindVertexArray(mesh_.va_);
		glDrawElements(GL_TRIANGLES, mesh_.index_count_, GL_UNSIGNED_INT, nullptr);
		glfwSwapBuffers(window_.getWindowHandle());
	}

	void DcApp::run()
	{
		int frames = 0, updates = 0;
		double constexpr dt = 1.0 / 60.0;
		double time = 0.0;
		double current_time = glfwGetTime();

		while (!glfwWindowShouldClose(window_.getWindowHandle()))
		{
			double const new_time = glfwGetTime();
			double frame_time = new_time - current_time;
			current_time = new_time;

			if (time > 1.0)
			{
				char frame_data[60];
				sprintf_s(frame_data, sizeof(frame_data), "FPS: %d, UPS: %d, Frametime: %f ms", frames, updates,
				          frame_time * 1000.0);
				window_.setTitle(frame_data);
				frames = 0;
				updates = 0;
				time = 0.0;
			}

			while (frame_time > 0.0)
			{
				double const delta_time = frame_time < dt ? frame_time : dt;
				update(static_cast<float>(delta_time));
				frame_time -= delta_time;
				time += delta_time;
				++updates;
			}
			render();
			++frames;
		}
	}
}
