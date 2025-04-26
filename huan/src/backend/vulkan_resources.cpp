//
// Created by 86156 on 4/21/2025.
//
#include "huan/backend/vulkan_resources.hpp"

#include "huan/HelloTriangleApplication.hpp"
#include "huan/backend/vulkan_buffer.hpp"
#include "huan/backend/vulkan_command.hpp"
#include "huan/backend/vulkan_image.hpp"
#include "huan/log/Log.hpp"

namespace huan
{

Scope<vulkan::Buffer> ResourceSystem::createBufferByStagingBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                                  vk::MemoryPropertyFlags memoryProperties,
                                                                  void* srcData)
{
    // 使用具备类实现私有构造函数
    struct EnableCreateScope : public vulkan::Buffer
    {
        explicit EnableCreateScope(Buffer& that) : Buffer(that)
        {
        }
    };

    vulkan::Buffer tempObj;
    tempObj.m_writeType = vulkan::Buffer::WriteType::Static;

    vk::BufferCreateInfo bufferInfo;
    bufferInfo.setSize(size)
              .setUsage(usage); // If you want to use a staging buffer to copy from

    // NOTE: 注意一定要使用 = {} 初始化，调用默认构造，否则会报错（出现无法预料的行为）
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    //因为我是Prefer_Device，所以会被隐式指定了Allow_Transfer_Instead_Bit, 于是也就需要去指定其它的标志位按照报错

    vmaCreateBuffer(allocatorHandle, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocationCreateInfo,
                    reinterpret_cast<VkBuffer*>(&tempObj.m_buffer), &tempObj.m_allocation,
                    nullptr);

    auto stagingBuffer = createBufferNormal(size, vk::BufferUsageFlagBits::eTransferSrc,
                                            srcData);

    copyBufferToBuffer(stagingBuffer->m_buffer, tempObj.m_buffer, size);
    destroyBuffer(stagingBuffer.get());

    if (!tempObj.m_buffer)
        HUAN_CORE_BREAK("Failed to create buffer.");

    return createScope<EnableCreateScope>(tempObj);
}

Scope<vulkan::Buffer> ResourceSystem::createBufferNormal(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                         void* srcData)
{
    // 使用具备类实现私有构造函数
    struct EnableCreateScope : public vulkan::Buffer
    {
        explicit EnableCreateScope(Buffer& that) : Buffer(that)
        {
        }
    };

    vulkan::Buffer tempObj;
    tempObj.m_writeType = vulkan::Buffer::WriteType::Dynamic;

    vk::BufferCreateInfo bufferInfo;
    bufferInfo.setSize(size)
              .setUsage(usage)
              .setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    vmaCreateBuffer(allocatorHandle, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocationCreateInfo,
                    reinterpret_cast<VkBuffer*>(&tempObj.m_buffer), &tempObj.m_allocation, nullptr);

    updateDataInBuffer(tempObj, srcData, size);

    if (!tempObj.m_buffer)
        HUAN_CORE_BREAK("[ResourceSystem]: Failed to create buffer");
    if (!tempObj.m_allocation)
        HUAN_CORE_BREAK("[ResourceSystem]: Failed to allocate memory! ");
    return createScope<EnableCreateScope>(tempObj);
}

void ResourceSystem::updateDataInBuffer(vulkan::Buffer& targetBuffer, void* srcData, vk::DeviceSize size,
                                        vk::DeviceSize srcOffset, vk::DeviceSize dstOffset)
{
    if (srcData == nullptr)
    {
        HUAN_CORE_ERROR("[ResourceSystem]: Failed to update data to buffer. Because the srcData is nullptr.");
        return;
    }
    if (targetBuffer.m_writeType == vulkan::Buffer::WriteType::Dynamic)
    {
        vmaCopyMemoryToAllocation(allocatorHandle, static_cast<char*>(srcData) + srcOffset, targetBuffer.m_allocation,
                                  dstOffset, size);
    }
    else if (targetBuffer.m_writeType == vulkan::Buffer::WriteType::Static)
    {
        auto stagingBuffer = createBufferNormal(size, vk::BufferUsageFlagBits::eTransferSrc,
                                                srcData);
        copyBufferToBuffer(targetBuffer.m_buffer, stagingBuffer->m_buffer, size);
        destroyBuffer(stagingBuffer.get());
    }
}

void ResourceSystem::updateDataInImage(vulkan::Image& targetImage, void* srcData, vk::DeviceSize size,
                                       vk::DeviceSize srcOffset, vk::DeviceSize dstOffset)
{
    if (srcData == nullptr)
    {
        HUAN_CORE_ERROR("Failed to update data to image. ")
        return;
    }
    if (targetImage.m_writeType == vulkan::Image::WriteType::Dynamic)
    {
        vmaCopyMemoryToAllocation(allocatorHandle, static_cast<char*>(srcData) + srcOffset, targetImage.m_allocation,
                                  dstOffset, size);
    }
    else if (targetImage.m_writeType == vulkan::Image::WriteType::Static)
    {
        auto stagingBuffer = createBufferNormal(size, vk::BufferUsageFlagBits::eTransferSrc,
                                                srcData);
        copyBufferToImage(stagingBuffer->m_buffer, targetImage.m_image, targetImage.m_extent);
        destroyBuffer(stagingBuffer.get());
    }
}

Scope<vulkan::Image> ResourceSystem::createImageByStagingBuffer(vk::ImageType imageType, const vk::Extent3D& extent,
                                                                uint32_t mipLevels, vk::Format format,
                                                                vk::ImageTiling tiling,
                                                                vk::ImageUsageFlags usage,
                                                                vk::MemoryPropertyFlags properties, void* data)
{
    class EnableCreateScope final : public vulkan::Image
    {
    public:
        explicit EnableCreateScope(vulkan::Image& that) : vulkan::Image(that)
        {

        }
    };

    vulkan::Image tempObj;
    tempObj.m_writeType = vulkan::Image::WriteType::Static;
    tempObj.m_extent = extent;

    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.setImageType(imageType)
                   .setExtent(extent)
                   .setMipLevels(mipLevels)
                   .setArrayLayers(1)
                   .setFormat(format)
                   .setTiling(tiling)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setUsage(usage)
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setSharingMode(vk::SharingMode::eExclusive)
                   .setQueueFamilyIndexCount(0)
                   .setPQueueFamilyIndices(nullptr)
                   .setFlags(vk::ImageCreateFlags());
    VmaAllocationCreateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    memoryAllocateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VmaAllocationInfo allocationInfo = {};
    vmaCreateImage(allocatorHandle, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &memoryAllocateInfo,
                   reinterpret_cast<VkImage*>(&tempObj.m_image), &tempObj.m_allocation, &allocationInfo);

    auto stagingBuffer = createBufferNormal(allocationInfo.size, vk::BufferUsageFlagBits::eTransferSrc,
                                            data);
    // Transition the layout to vk::ImageLayout::eTransferDstOptimal for copying data to it.
    transitionImageLayout(tempObj.m_image, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    copyBufferToImage(stagingBuffer->m_buffer, tempObj.m_image, extent);
    // Transition the layout to vk::ImageLayout::eShaderReadOnlyOptimal for shaders using it.
    transitionImageLayout(tempObj.m_image, format, vk::ImageLayout::eTransferDstOptimal,
                          vk::ImageLayout::eShaderReadOnlyOptimal);
    destroyBuffer(stagingBuffer.get());

    return createScope<EnableCreateScope>(tempObj);
}

Scope<vulkan::Image> ResourceSystem::createImageNormal(vk::ImageType imageType, const vk::Extent3D& extent,
                                                       uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling,
                                                       vk::ImageUsageFlags usage,
                                                       vk::MemoryPropertyFlags properties, void* data)
{
    class EnableCreateScope final : public vulkan::Image
    {
    public:
        explicit EnableCreateScope(vulkan::Image& that) : vulkan::Image(that)
        {

        }
    };

    vulkan::Image tempObj;
    tempObj.m_writeType = vulkan::Image::WriteType::Dynamic;
    tempObj.m_extent = extent;

    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.setImageType(imageType)
                   .setExtent(extent)
                   .setMipLevels(mipLevels)
                   .setArrayLayers(1)
                   .setFormat(format)
                   .setTiling(tiling)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setUsage(usage)
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setSharingMode(vk::SharingMode::eExclusive)
                   .setQueueFamilyIndexCount(0)
                   .setPQueueFamilyIndices(nullptr)
                   .setFlags(vk::ImageCreateFlags());
    
    VmaAllocationCreateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    VmaAllocationInfo allocationInfo;
    vmaCreateImage(allocatorHandle, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &memoryAllocateInfo,
                   reinterpret_cast<VkImage*>(&tempObj.m_image), &tempObj.m_allocation, &allocationInfo);

    updateDataInImage(tempObj, data, allocationInfo.size);

    return createScope<EnableCreateScope>(tempObj);
}

vk::ImageView ResourceSystem::createImageView(vk::Image image, vk::ImageViewType viewType, vk::Format format,
                                              vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    vk::ImageViewCreateInfo imageViewCreateInfo;
    imageViewCreateInfo.setImage(image)
                       .setViewType(viewType)
                       .setFormat(format)
                       .setSubresourceRange(vk::ImageSubresourceRange(aspectFlags, 0, mipLevels, 0, 1));

    return deviceHandle.createImageView(imageViewCreateInfo);
}

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

    vk::ImageMemoryBarrier barrier;
    barrier.setOldLayout(oldLayout)
           .setNewLayout(newLayout)
           .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored) // 如果你不想转移队列族所有权，你可以指定为ignored
           .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
           .setImage(image)
           .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    // Only use an image memory barrier
    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.setSrcAccessMask({})
               .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
               .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::ePresentSrcKHR)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
               .setDstAccessMask(vk::AccessFlagBits::eMemoryRead);

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eBottomOfPipe;
    }
    else
    {
        HUAN_CORE_BREAK("[ResourceSystem]: Unsupported layout transition! ")
    }

    commandBuffer.pipelineBarrier(srcStage, dstStage,
                                  vk::DependencyFlags{},
                                  nullptr, nullptr, barrier);

    commandSystem->endSingleTimeCommands(commandPool, commandBuffer);
}

void ResourceSystem::copyBufferToBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    // TODO: With queue family info, dynamically select the commandPool
    auto commandSystem = CommandSystem::getInstance();
    auto commandPool = HelloTriangleApplication::getInstance()->m_commandPool;
    auto commandBuffer = commandSystem->beginSingleTimeCommands(commandPool);

    vk::BufferCopy region;
    region.setSize(size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &region);

    commandSystem->endSingleTimeCommands(commandPool, commandBuffer);
}

void ResourceSystem::copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, vk::Extent3D extent)
{
    auto commandSystem = CommandSystem::getInstance();
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

void ResourceSystem::destroyBuffer(vulkan::Buffer* buffer)
{
    vmaDestroyBuffer(allocatorHandle, buffer->m_buffer, buffer->m_allocation);
}

void ResourceSystem::destroyImage(vulkan::Image* image)
{
    vmaDestroyImage(allocatorHandle, image->m_image, image->m_allocation);
}

ResourceSystem::ResourceSystem()
    : deviceHandle(HelloTriangleApplication::getInstance()->device),
      physicalDeviceHandle(HelloTriangleApplication::getInstance()->physicalDevice),
      allocatorHandle(HelloTriangleApplication::getInstance()->allocator)
{

}
}