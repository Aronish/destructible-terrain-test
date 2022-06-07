#include "fence_wrapper.hpp"

namespace eng
{
    GpuFenceManager::~GpuFenceManager()
    {
        for (auto const & fence : m_fences) glDeleteSync(fence.first);
    }

    void GpuFenceManager::update()
    {   
        auto iterator = m_fences.begin();
        while (iterator != m_fences.end())
        {
            GLint result[1];
            glGetSynciv((*iterator).first, GL_SYNC_STATUS, sizeof(result), nullptr, result);
            if (result[0] == GL_SIGNALED)
            {
                (*iterator).second();
                glDeleteSync((*iterator).first);
                iterator = m_fences.erase(iterator);
                continue;
            }
            ++iterator;
        }
    }

    void GpuFenceManager::setBarrier(CompleteCallback completed_action)
    {
        m_fences.insert({ glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0) , completed_action });
        glFlush();
    }
}