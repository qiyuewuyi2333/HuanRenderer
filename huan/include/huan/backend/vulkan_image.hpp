//
// Created by 86156 on 4/20/2025.
//

#ifndef VULKAN_IMAGE_HPP
#define VULKAN_IMAGE_HPP
#include <vulkan/vulkan.hpp>

#include "vk_mem_alloc.h"
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
    VmaAllocation m_allocation;
    WriteType m_writeType;
    vk::Extent3D m_extent;

protected:
    Image();
    Image(Image& that);
};

}

#endif //VULKAN_IMAGE_HPP