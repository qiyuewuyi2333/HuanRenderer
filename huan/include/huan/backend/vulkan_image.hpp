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
/**
 * 如果你想设置当前Image的mipmap 为 1，只需要调用  setMipmapCount(1) 然后我们会自动重新生成 ImageView
 */
class Image
{
    friend class ::huan::ResourceSystem;

public:
    enum class WriteType
    {
        Static,
        Dynamic
    };
    // TODO: setBaseMipmapLevel 动态生成imageView 后续可能会增加缓存什么的 如果需要高速更改的话

INNER_VISIBLE:
    vk::Image m_image;
    vk::ImageView m_imageView;
    VmaAllocation m_allocation;
    WriteType m_writeType;
    vk::Extent3D m_extent;

    /**
     * 方便我们后续动态调整View
     */
    vk::ImageViewCreateInfo m_viewInfo;

protected:
    Image();
    Image(Image& that);
};

}

#endif //VULKAN_IMAGE_HPP