#pragma once

#include <functional>
#include <unordered_map>

#include <glad/glad.h>

namespace eng
{
    class GpuFenceManager
    {
    private:
        using CompleteCallback = std::function<void()>;

        std::unordered_map<GLsync, CompleteCallback> m_fences;

    public:
        ~GpuFenceManager();

        void update();
        void setBarrier(CompleteCallback completed_action);
    };
}