//
// Created by 86156 on 4/21/2025.
//

#ifndef VULKAN_RESOURCES_HPP
#define VULKAN_RESOURCES_HPP

#include "vk_mem_alloc.h"
#include "huan/common.hpp"
#include "huan/common_templates/deferred_system.hpp"
#include "huan/log/Log.hpp"
#include "vulkan/vulkan.hpp"

namespace huan::vulkan
{
class Image;
class Buffer;
}

namespace huan
{

class ResourceSystem final : public DeferredSystem<ResourceSystem>
{
    friend class DeferredSystem<ResourceSystem>;

public:
    Scope<vulkan::Buffer> createBufferByStagingBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                      vk::MemoryPropertyFlags memoryProperties,
                                                      void* srcData = nullptr);
    Scope<vulkan::Buffer> createBufferNormal(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                             void* srcData = nullptr);
    void updateDataInBuffer(vulkan::Buffer& targetBuffer, void* srcData, vk::DeviceSize size,
                            vk::DeviceSize srcOffset = 0, vk::DeviceSize dstOffset = 0);
    void updateDataInImage(vulkan::Image& targetImage, void* srcData, vk::DeviceSize size,
                           vk::DeviceSize srcOffset = 0, vk::DeviceSize dstOffset = 0);


    Scope<vulkan::Image> createImageDeviceLocal(vk::ImageType imageType, const vk::Extent3D& extent,
                                                    uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling,
                                                    vk::ImageUsageFlags usage,
                                                    vk::MemoryPropertyFlags properties, void* data = nullptr);
    Scope<vulkan::Image> createImageNormal(vk::ImageType imageType, const vk::Extent3D& extent, uint32_t mipLevels,
                                           vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                           vk::MemoryPropertyFlags properties, void* data = nullptr);
    void createImageView(vulkan::Image& image, vk::ImageViewType viewType, vk::Format format,
                                  vk::ImageAspectFlags aspectFlags,
                                  uint32_t mipLevels);
    uint32_t findRequiredMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout);
    bool hasStencilComponent(vk::Format format) const;
    bool isDepthStencilFormat(vk::Format format) const;

    void copyBufferToBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
    void copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, vk::Extent3D extent);

    void destroyBuffer(vulkan::Buffer* buffer);
    void destroyImage(vulkan::Image* image);

protected:
    explicit ResourceSystem();

    vk::Device& deviceHandle;
    vk::PhysicalDevice& physicalDeviceHandle;
    VmaAllocator& allocatorHandle;

};

}


#endif //VULKAN_RESOURCES_HPP