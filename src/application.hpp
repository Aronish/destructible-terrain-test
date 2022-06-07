#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <PxPhysicsAPI.h>

#include "event/event.hpp"
#include "first_person_camera.hpp"
#include "graphics/asset.hpp"
#include "graphics/fence_wrapper.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "graphics/vertex_array.hpp"
#include "window.hpp"
#include "world/world.hpp"
#include "world/chunk.hpp"

namespace eng
{
    class Application
    {
    private:
        class EngPxAllocatorCallback : public physx::PxAllocatorCallback
        {
        public:
            void * allocate(size_t size, char const *, char const *, int) override
            {
                return _aligned_malloc(size, 16);
            }

            void deallocate(void * ptr) override
            {
                _aligned_free(ptr);
            }
        } m_px_allocator_callback;

        class EngPxErrorCallback : public physx::PxErrorCallback
        {
        public:
            void reportError(physx::PxErrorCode::Enum code, char const * message, char const *, int) override
            {
                ENG_LOG_F("[PhysX] %d: %s", code, message);
            }
        } m_px_error_callback;

        physx::PxFoundation * m_px_foundation;
        physx::PxPhysics * m_px_physics;

        Window m_window; // Has to be first due to OpenGL initialization
        AssetManager m_asset_manager;
        GpuFenceManager m_gpu_fence_manager;
        FirstPersonCamera m_camera;
        World m_world;

        std::shared_ptr<Texture> m_crosshair_texture;
        std::shared_ptr<Shader> m_textured_quad_shader;
        GLuint m_crosshair_va;
        GLuint m_crosshair_vb;
        GLuint m_crosshair_ib;

    public:
        Application(unsigned int width, unsigned int height, char const * title, bool maximized);
        ~Application();

        void run();
        void onEvent(Event const & event);
        void update(float delta_time);
        void render();
    };
}