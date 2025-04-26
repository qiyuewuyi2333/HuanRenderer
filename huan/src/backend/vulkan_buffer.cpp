//
// Created by 86156 on 4/18/2025.
//

#include <strstream>
#include <huan/backend/vulkan_buffer.hpp>
#include "huan/common.hpp"
#include "huan/backend/vulkan_resources.hpp"

namespace huan::vulkan
{

Buffer::~Buffer()
{
    // Do nothing
    // NOTE: If you want to release the resources, please invoke the ResourceSystem to destroy it.
}

Buffer::Buffer(Buffer& that)
{
    if (this != &that) [[likely]]
    {
        std::swap(m_buffer, that.m_buffer);
        std::swap(m_allocation, that.m_allocation);
        std::swap(m_writeType, that.m_writeType);
        std::swap(m_Init, that.m_Init);
    }
}

}