//
// Created by 86156 on 4/20/2025.
//

#ifndef VULKAN_IMAGE_HPP
#define VULKAN_IMAGE_HPP
#include <vulkan/vulkan.hpp>

#include "huan/common.hpp"

namespace huan
{
class ResourceSystem;
}

namespace huan::vulkan
{
class Image
{
    friend class ::huan::ResourceSystem;

public:
    enum class WriteType
    {
        Static,
        Dynamic
    };

INNER_VISIBLE:
    vk::Image m_image;
    vk::DeviceMemory m_memory;
    WriteType m_writeType;
    vk::Extent3D m_extent;
    void* m_data = nullptr;

protected:
    Image();
    Image(Image& that);
};

}

#endif //VULKAN_IMAGE_HPP