//
// Created by 86156 on 4/18/2025.
//

#ifndef VULKANBUFFER_HPP
#define VULKANBUFFER_HPP

#include "vulkan/vulkan.hpp"
#include "huan/HelloTriangleApplication.hpp"
#include "huan/log/Log.hpp"

namespace huan
{
class ResourceSystem;
}

namespace huan::vulkan
{
class Buffer
{
    friend class ::huan::ResourceSystem;

public:
    enum class WriteType
    {
        Static,
        Dynamic
    };

    ~Buffer();

INNER_VISIBLE:
    vk::Buffer m_buffer;
    vk::DeviceMemory m_memory;
    void* m_data = nullptr;
    WriteType m_writeType;

private:
    Buffer() = default;
    Buffer(Buffer& that);
    Buffer& operator=(Buffer& that) = delete;
};
}

#endif //VULKANBUFFER_HPP