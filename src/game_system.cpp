#include "game_system.hpp"

namespace eng
{
    GameSystem::GameSystem()
    {
#pragma warning(push)
#pragma warning(disable : 6011)
        m_px_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_px_allocator_callback, m_px_error_callback);
        if (!m_px_foundation) ENG_LOG("Failed to initialize PxFoundation!");

        m_px_pvd = physx::PxCreatePvd(*m_px_foundation);

        m_pvd_transport = physx::PxDefaultPvdSocketTransportCreate("87.92.164.212", 5425, 10);
        m_px_pvd->connect(*m_pvd_transport, physx::PxPvdInstrumentationFlag::eALL);

        m_px_physics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *m_px_foundation, physx::PxTolerancesScale(), true, m_px_pvd);
        if (!m_px_physics) ENG_LOG("Failed to initialize PxPhysics!");

        if(!PxInitExtensions(*m_px_physics, m_px_pvd)) ENG_LOG("Failed to initialize Physx Extensions!");
#pragma warning(pop)

        m_px_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_px_foundation, physx::PxCookingParams(physx::PxTolerancesScale()));
        if (!m_px_cooking) ENG_LOG("Failed to initialize PxCooking!");

        m_px_cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(1);
    }
    
    GameSystem::~GameSystem()
    {
        m_px_physics->release();
        m_px_cooking->release();
        PxCloseExtensions();
        m_px_pvd->release();
        m_pvd_transport->release();
        m_px_foundation->release(); // Release last
    }
    
    physx::PxFoundation * GameSystem::getPhysxFoundation() const
    {
        return m_px_foundation;
    }

    physx::PxPhysics * GameSystem::getPhysx() const
    {
        return m_px_physics;
    }

    physx::PxCooking * GameSystem::getPhysxCooking() const
    {
        return m_px_cooking;
    }
  
    physx::PxCpuDispatcher * GameSystem::getPhysxCpuDispatcher() const
    {
        return m_px_cpu_dispatcher;
    }
  
    AssetManager & GameSystem::getAssetManager()
    {
        return m_asset_manager;
    }

    GpuSynchronizer & GameSystem::getGpuSynchronizer()
    {
        return m_gpu_synchronizer;
    }
}