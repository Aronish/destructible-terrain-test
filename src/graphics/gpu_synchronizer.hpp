#pragma once

#include <functional>
#include <unordered_map>

#include <glad/glad.h>

namespace eng
{
    class GpuSynchronizer
    {
    private:
        using CompleteCallback = std::function<void()>;
        using ReadCallback = std::function<void(std::vector<float> const &)>;

        std::unordered_map<GLsync, CompleteCallback> m_fences;

    public:
        ~GpuSynchronizer();

        void update();
        void setBarrier(CompleteCallback completed_action);
        void readBufferWhenReady(GLuint buffer, GLintptr offset, GLsizeiptr size, ReadCallback completed_action);
    };
}