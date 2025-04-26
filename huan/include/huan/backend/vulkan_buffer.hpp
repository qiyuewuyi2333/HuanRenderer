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
    VmaAllocation m_allocation;
    WriteType m_writeType;
    bool m_Init = false;

private:
    Buffer() = default;
    Buffer(Buffer& that);
    Buffer& operator=(Buffer& that) = delete;
};
}

#endif //VULKANBUFFER_HPP