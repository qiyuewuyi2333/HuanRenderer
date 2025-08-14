//
// Created by 86156 on 4/21/2025.
//
#include "huan/backend/resource/resource_system.hpp"

#include "huan/HelloTriangleApplication.hpp"
#include "huan/backend/vulkan_command.hpp"
#include "huan/backend/resource/vulkan_image_view.hpp"
#include "huan/log/Log.hpp"

namespace huan::runtime
{

Scope<vulkan::Buffer> ResourceSystem::createStagingBuffer(vk::BufferUsageFlags usage,
                                                          vk::DeviceSize size,
                                                          const void* srcData)
{
    vulkan::BufferBuilder builder(allocatorHandle, size);
    builder.setVmaFlags(VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
           .setUsage(vk::BufferUsageFlagBits::eTransferSrc | usage);
    auto res = builder.buildScope(deviceHandle);
    if (srcData != nullptr)
    {
        res->updateWithMapping(srcData, size);
    }

    return res;
}

Scope<vulkan::Buffer> ResourceSystem::createDeviceLocalBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size,
                                                              void* srcData)
{
    // 创建 device local buffer
    vulkan::BufferBuilder builder(allocatorHandle, size);
    builder.setVmaUsage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
           .setUsage(usage | vk::BufferUsageFlagBits::eTransferDst);

    auto deviceBuffer = builder.buildScope(deviceHandle);

    // 如果有源数据，通过 staging buffer 传输
    if (srcData != nullptr)
    {
        executeImmediateTransfer([&](vk::CommandBuffer cmd) {
            // 创建临时 staging buffer
            auto stagingBuffer = createStagingBuffer(usage, size, srcData);

            // 记录复制命令
            vk::BufferCopy copyRegion{};
            copyRegion.size = size;
            cmd.copyBuffer(stagingBuffer->getHandle(), deviceBuffer->getHandle(), copyRegion);
            m_deletingBufferQueue.emplace(std::move(stagingBuffer));
        });
    }
    return deviceBuffer;
}

Scope<vulkan::Image> ResourceSystem::createImage(vk::ImageType imageType, const vk::Extent3D& extent,
                                                 uint32_t mipLevels,
                                                 vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                                 vk::MemoryPropertyFlags properties,
                                                 void* data)
{
    vulkan::ImageBuilder builder(allocatorHandle, extent);
    builder.setImageType(imageType)
           .setMipLevels(mipLevels)
           .setFormat(format)
           .setTiling(tiling)
           .setUsage(usage)
           .setVmaPreferredFlags(properties);

    auto image = builder.buildUnique(deviceHandle);
    vk::MemoryRequirements memoryRequirements{};
    deviceHandle.getImageMemoryRequirements(image->getHandle(), &memoryRequirements);

    if (data != nullptr)
    {
        auto stagingBuffer = createStagingBuffer(vk::BufferUsageFlagBits::eTransferSrc, memoryRequirements.size, data);
        transitionImageLayout(image->getHandle(), format, vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eTransferDstOptimal);
        copyBufferToImage(stagingBuffer->getHandle(), image->getHandle(), extent);
        transitionImageLayout(image->getHandle(), format, vk::ImageLayout::eTransferDstOptimal,
                              vk::ImageLayout::eShaderReadOnlyOptimal);
        stagingBuffer.reset();
    }

    return image;
}


// Scope<vulkan::Image> ResourceSystem::createImageDeviceLocal(vk::ImageType imageType, const vk::Extent3D& extent,
//                                                             uint32_t mipLevels, vk::Format format,
//                                                             vk::ImageTiling tiling, vk::ImageUsageFlags usage,
//                                                             vk::MemoryPropertyFlags properties, void* data)
// {
//     class EnableCreateScope final : public vulkan::Image
//     {
//     public:
//         explicit EnableCreateScope(vulkan::Image& that) : vulkan::Image(that)
//         {
//         }
//     };
//
//     vulkan::Image tempObj;
//     tempObj.m_writeType = vulkan::Image::WriteType::Static;
//     tempObj.m_extent = extent;
//
//     vk::ImageCreateInfo imageCreateInfo;
//     imageCreateInfo.setImageType(imageType)
//                    .setExtent(extent)
//                    .setMipLevels(mipLevels)
//                    .setArrayLayers(1)
//                    .setFormat(format)
//                    .setTiling(tiling)
//                    .setInitialLayout(vk::ImageLayout::eUndefined)
//                    .setUsage(usage)
//                    .setSamples(vk::SampleCountFlagBits::e1)
//                    .setSharingMode(vk::SharingMode::eExclusive)
//                    .setQueueFamilyIndexCount(0)
//                    .setPQueueFamilyIndices(nullptr)
//                    .setFlags(vk::ImageCreateFlags());
//     VmaAllocationCreateInfo memoryAllocateInfo = {};
//     memoryAllocateInfo.usage = VMA_MEMORY_USAGE_AUTO;
//     memoryAllocateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//     VmaAllocationInfo allocationInfo = {};
//     vmaCreateImage(allocatorHandle, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &memoryAllocateInfo,
//                    reinterpret_cast<VkImage*>(&tempObj.m_image), &tempObj.m_allocation, &allocationInfo);
//
//     if (data)
//     {
//         auto stagingBuffer = createBufferNormal(allocationInfo.size, vk::BufferUsageFlagBits::eTransferSrc, data);
//         // Transition the layout to vk::ImageLayout::eTransferDstOptimal for copying data to it.
//         transitionImageLayout(tempObj.m_image, format, vk::ImageLayout::eUndefined,
//                               vk::ImageLayout::eTransferDstOptimal);
//         copyBufferToImage(stagingBuffer->m_buffer, tempObj.m_image, extent);
//         // Transition the layout to vk::ImageLayout::eShaderReadOnlyOptimal for shaders using it.
//         transitionImageLayout(tempObj.m_image, format, vk::ImageLayout::eTransferDstOptimal,
//                               vk::ImageLayout::eShaderReadOnlyOptimal);
//         destroyBuffer(stagingBuffer.get());
//     }
//
//     return createScope<EnableCreateScope>(tempObj);
// }

/**
 * 获取满足类型筛选(typeFilter)和目标内存属性properties的内存类型索引
 * @param typeFilter
 * @param properties
 * @return
 */
uint32_t ResourceSystem::findRequiredMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDeviceHandle.getMemoryProperties();
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (i << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    HUAN_CORE_BREAK("Failed to find suitable memory type! ")
    return 0;
}

/**
 * 
 * @param image 
 * @param imageViewType 
 * @param format 
 * @param mipLevels 生成的MipLevel的级数
 */
vulkan::ImageView* ResourceSystem::createImageView(vulkan::Image& image, vk::ImageViewType imageViewType,
                                                  vk::Format format,
                                                  uint32_t mipLevels)
{
    return new vulkan::ImageView{image, imageViewType, format, mipLevels};
}

/**
 * Using the image memory barrier to synchronize access to resources
 * @param image
 * @param format using in depth-buffer transition
 * @param oldLayout
 * @param newLayout
 */
void ResourceSystem::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                                           vk::ImageLayout newLayout)
{
    auto commandSystem = CommandSystem::getInstance();
    auto commandPool = HelloTriangleApplication::getInstance()->m_commandPool;
    auto commandBuffer = commandSystem->beginSingleTimeCommands(commandPool);

    vk::ImageMemoryBarrier barrier = {};
    barrier.setOldLayout(oldLayout)
           .setNewLayout(newLayout)
           .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored) // 如果你不想转移队列族所有权，你可以指定为ignored
           .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
           .setImage(image)
           .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

    if (isDepthStencilFormat(format))
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    // Only use an image memory barrier
    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.setSrcAccessMask({}).setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite).setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::ePresentSrcKHR)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite).setDstAccessMask(vk::AccessFlagBits::eMemoryRead);

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eBottomOfPipe;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthAttachmentOptimal)
    {
        barrier.setSrcAccessMask({}).setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead |
                                                      vk::AccessFlagBits::eDepthStencilAttachmentWrite);
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else
    {
        HUAN_CORE_BREAK("[ResourceSystem]: Unsupported layout transition! ")
    }

    commandBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags{}, nullptr, nullptr, barrier);

    commandSystem->endSingleTimeCommands(commandPool, commandBuffer);
}

bool ResourceSystem::hasStencilComponent(const vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

bool ResourceSystem::isDepthStencilFormat(vk::Format format)
{
    return format == vk::Format::eD16Unorm || format == vk::Format::eD32Sfloat ||
           format == vk::Format::eD24UnormS8Uint || format == vk::Format::eD32SfloatS8Uint ||
           format == vk::Format::eD16UnormS8Uint;
}

void ResourceSystem::copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, vk::Extent3D extent)
{
    const auto commandSystem = CommandSystem::getInstance();
    auto commandPool = HelloTriangleApplication::getInstance()->m_commandPool;
    auto commandBuffer = commandSystem->beginSingleTimeCommands(commandPool);

    vk::BufferImageCopy region;
    region.setBufferOffset(0)
          .setBufferRowLength(0)
          // NOTE: bufferRowLength and bufferImageHeight indicate the layout of the pixel data in the buffer
          .setBufferImageHeight(0)
          .setImageSubresource(vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1})
          .setImageOffset({0, 0, 0})
          .setImageExtent(extent);

    commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, region);

    commandSystem->endSingleTimeCommands(commandPool, commandBuffer);
}

// void ResourceSystem::destroyBuffer(vulkan::Buffer* buffer)
// {
//     vmaDestroyBuffer(allocatorHandle, buffer->m_buffer, buffer->m_allocation);
// }
//
// void ResourceSystem::destroyImage(vulkan::Image* image)
// {
//     if (image->m_imageView)
//         deviceHandle.destroy(image->m_imageView);
//     vmaDestroyImage(allocatorHandle, image->m_image, image->m_allocation);
// }

ResourceSystem::ResourceSystem()
    : deviceHandle(HelloTriangleApplication::getInstance()->device),
      physicalDeviceHandle(HelloTriangleApplication::getInstance()->physicalDevice),
      allocatorHandle(HelloTriangleApplication::getInstance()->allocator)
{
}
} // namespace huan