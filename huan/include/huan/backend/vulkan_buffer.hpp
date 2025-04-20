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
struct VulkanBuffer
{
    enum class WriteType
    {
        Static,
        Dynamic
    };

    void copyFrom(vk::Buffer buffer, VkDeviceSize size);

    vk::Buffer m_buffer;
    vk::DeviceMemory m_memory;
    void* m_data = nullptr;
    WriteType m_writeType;

    vk::Device& deviceHandle = HelloTriangleApplication::getInstance()->device;

    ~VulkanBuffer()
    {
        if (m_buffer != nullptr)
        {
            deviceHandle.destroyBuffer(m_buffer);
        }
        if (m_memory != nullptr)
        {
            deviceHandle.freeMemory(m_memory);
        }
    }

private:
    VulkanBuffer() = default;
    VulkanBuffer(VulkanBuffer& that);
    VulkanBuffer& operator=(VulkanBuffer& that) = delete;


    struct PrivateKey
    {
        explicit PrivateKey() = default;
    };

private:
    static Scope<VulkanBuffer> createByStagingBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                     vk::MemoryPropertyFlags memoryProperties, void* srcData = nullptr);
    static Scope<VulkanBuffer> createNormal(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                            vk::MemoryPropertyFlags memoryProperties, void* srcData = nullptr);

public:
    void updateData(void* srcData, VkDeviceSize size, VkDeviceSize offset = 0);
    static Scope<VulkanBuffer> create(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                      vk::MemoryPropertyFlags memoryProperties, void* srcData = nullptr);
    static uint32_t findRequiredMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
};
}

#endif //VULKANBUFFER_HPP