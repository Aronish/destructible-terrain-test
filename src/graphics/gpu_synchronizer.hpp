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

        template<typename DataType> requires std::is_arithmetic_v<DataType>
        using ReadCallback = std::function<void(std::vector<DataType> const &)>;

        std::unordered_map<GLsync, CompleteCallback> m_fences;

    public:
        ~GpuSynchronizer();

        void update();
        void setBarrier(CompleteCallback completed_action);

        template<typename DataType> requires std::is_arithmetic_v<DataType>
        void readBufferWhenReady(GLuint buffer, GLintptr offset, GLsizeiptr size, ReadCallback<DataType> const & completed_action)
        {
            setBarrier([=]()
            {
                std::vector<DataType> output(static_cast<size_t>(size) / sizeof(DataType));
                glGetNamedBufferSubData(buffer, offset, size, output.data());
                completed_action(output);
            });
        }
    };
}