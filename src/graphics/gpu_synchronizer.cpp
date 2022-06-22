#include <vector>

#include "logger.hpp"

#include "gpu_synchronizer.hpp"

namespace eng
{
    GpuSynchronizer::~GpuSynchronizer()
    {
        for (auto const & fence : m_fences) glDeleteSync(fence.first);
    }

    void GpuSynchronizer::update()
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

    void GpuSynchronizer::setBarrier(CompleteCallback completed_action)
    {
        m_fences.insert({ glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0) , completed_action });
        glFlush();
    }

    void GpuSynchronizer::readBufferWhenReady(GLuint buffer, GLintptr offset, GLsizeiptr size, ReadCallback completed_action)
    {
        setBarrier([=]()
        {
            std::vector<float> output(static_cast<size_t>(size) / sizeof(float));
            glGetNamedBufferSubData(buffer, offset, size, output.data());
            completed_action(output);
        });
    }
}