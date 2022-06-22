#pragma once

#include <PxPhysicsAPI.h>

#include "graphics/asset.hpp"
#include "graphics/gpu_synchronizer.hpp"
#include "logger.hpp"

namespace eng
{
    class GameSystem
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

    private:
        physx::PxFoundation * m_px_foundation;
        physx::PxPhysics * m_px_physics;
        physx::PxCooking * m_px_cooking;
        physx::PxPvd * m_px_pvd;
        physx::PxCpuDispatcher * m_px_cpu_dispatcher;
        AssetManager m_asset_manager;
        GpuSynchronizer m_gpu_synchronizer;

    public:
        GameSystem();
        ~GameSystem();

        physx::PxFoundation * getPhysxFoundation() const;
        physx::PxPhysics * getPhysx() const;
        physx::PxCooking * getPhysxCooking() const;
        physx::PxCpuDispatcher * getPhysxCpuDispatcher() const;

        AssetManager & getAssetManager();
        GpuSynchronizer & getGpuSynchronizer();
    };
}