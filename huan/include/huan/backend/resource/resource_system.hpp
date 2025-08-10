//
// Created by 86156 on 4/21/2025.
//

#ifndef VULKAN_RESOURCES_HPP
#define VULKAN_RESOURCES_HPP

#include "vk_mem_alloc.h"
#include "huan/HelloTriangleApplication.hpp"
#include "huan/common.hpp"
#include "huan/common_templates/deferred_system.hpp"
#include "huan/log/Log.hpp"
#include "vulkan/vulkan.hpp"
#include "huan/backend/resource/vulkan_buffer.hpp"

namespace huan::runtime::vulkan
{
class Image;
} // namespace huan::vulkan

namespace huan::runtime
{

class ResourceSystem final : public DeferredSystem<ResourceSystem>
{
    friend class DeferredSystem<ResourceSystem>;

public:

#pragma region Buffer

#pragma region 创建StagingBuffer
    vulkan::Buffer createStagingBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size,
                                       const void* srcData = nullptr);
    template <class T>
    vulkan::Buffer createStagingBuffer(vk::BufferUsageFlags usage, const vk::ArrayProxy<T>& srcData);
#pragma endregion

#pragma region 创建DeviceLocalBuffer
    vulkan::Buffer createDeviceLocalBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size,
                                           void* srcData = nullptr);
    template <class T>
    vulkan::Buffer createDeviceLocalBuffer(vk::BufferUsageFlags usage, const vk::ArrayProxy<T>& srcData);

#pragma endregion
    template <typename Func>
    void executeImmediateTransfer(Func&& func) const;

#pragma endregion
    void updateDataInBuffer(vulkan::Buffer& targetBuffer, void* srcData, vk::DeviceSize size,
                            vk::DeviceSize srcOffset = 0, vk::DeviceSize dstOffset = 0);

    void updateDataInImage(vulkan::Image& targetImage, void* srcData, vk::DeviceSize size, vk::DeviceSize srcOffset = 0,
                           vk::DeviceSize dstOffset = 0);

    Scope<vulkan::Image> createImageDeviceLocal(vk::ImageType imageType, const vk::Extent3D& extent, uint32_t mipLevels,
                                                vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                                vk::MemoryPropertyFlags properties, void* data = nullptr);
    Scope<vulkan::Image> createImageNormal(vk::ImageType imageType, const vk::Extent3D& extent, uint32_t mipLevels,
                                           vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                           vk::MemoryPropertyFlags properties, void* data = nullptr);
    void createImageView(vulkan::Image& image, vk::ImageViewType viewType, vk::Format format,
                         vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
    uint32_t findRequiredMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout);
    [[nodiscard]] bool hasStencilComponent(vk::Format format) const;
    [[nodiscard]] bool isDepthStencilFormat(vk::Format format) const;

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

template <class T>
vulkan::Buffer ResourceSystem::createStagingBuffer(vk::BufferUsageFlags usage, const vk::ArrayProxy<T>& srcData)
{
    return createStagingBuffer(usage, srcData.size() * sizeof(T), &srcData);
}

class ScopedCommandBuffer
{
    vk::Device device;
    vk::CommandPool pool;
    vk::CommandBuffer cmd;

public:
    ScopedCommandBuffer(vk::Device dev, vk::CommandPool p)
        : device(dev), pool(p)
    {
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = pool;
        allocInfo.commandBufferCount = 1;

        cmd = device.allocateCommandBuffers(allocInfo)[0];

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        cmd.begin(beginInfo);
    }

    ~ScopedCommandBuffer()
    {
        if (cmd)
        {
            device.freeCommandBuffers(pool, cmd);
        }
    }

    vk::CommandBuffer get() const
    {
        return cmd;
    }

    void submitAndWait(vk::Queue queue)
    {
        cmd.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        queue.submit(submitInfo);
        queue.waitIdle();
    }

    // 禁止复制
    ScopedCommandBuffer(const ScopedCommandBuffer&) = delete;
    ScopedCommandBuffer& operator=(const ScopedCommandBuffer&) = delete;
};

template <class T>
vulkan::Buffer ResourceSystem::createDeviceLocalBuffer(vk::BufferUsageFlags usage, const vk::ArrayProxy<T>& srcData)
{
    return createDeviceLocalBuffer(usage, srcData.size() * sizeof(T), &srcData);
}

template <typename Func>
void ResourceSystem::executeImmediateTransfer(Func&& func) const
{
    ScopedCommandBuffer scopedCmd(deviceHandle, HelloTriangleApplication::instance->m_transferCommandPool);
    func(scopedCmd.get());
    scopedCmd.submitAndWait(HelloTriangleApplication::instance->transferQueue);
}

} // namespace huan

#endif // VULKAN_RESOURCES_HPP