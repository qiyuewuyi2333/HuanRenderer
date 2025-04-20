//
// Created by 86156 on 4/18/2025.
//

#include <strstream>
#include <huan/backend/vulkan_buffer.hpp>
#include "huan/common.hpp"

namespace huan
{


void VulkanBuffer::copyFrom(vk::Buffer buffer, VkDeviceSize size)
{
    // 创建Buffer使用的CommandPool 由某个命令队列创建
    // 而这个命令队列 必须要和提交CommandBuffer使用的队列一致
    vk::CommandBufferAllocateInfo cmdBufferAllocInfo;
    cmdBufferAllocInfo.setCommandPool(HelloTriangleApplication::getInstance()->m_transferCommandPool)
                      .setCommandBufferCount(1)
                      .setLevel(vk::CommandBufferLevel::ePrimary);

    vk::CommandBuffer copyCmdBuffer = deviceHandle.allocateCommandBuffers(cmdBufferAllocInfo)[0];
    vk::CommandBufferBeginInfo cmdBeginInfo;
    cmdBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    copyCmdBuffer.begin(cmdBeginInfo);
    vk::BufferCopy copyRegion;
    copyRegion.setSize(size)
              .setSrcOffset(0)
              .setDstOffset(0);
    copyCmdBuffer.copyBuffer(buffer, m_buffer, copyRegion);
    copyCmdBuffer.end();
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(copyCmdBuffer);

    auto transferQueue = HelloTriangleApplication::getInstance()->transferQueue;
    transferQueue.submit(submitInfo, nullptr);
    transferQueue.waitIdle();
    deviceHandle.freeCommandBuffers(HelloTriangleApplication::getInstance()->m_transferCommandPool, copyCmdBuffer);
}

VulkanBuffer::VulkanBuffer(VulkanBuffer& that)
{
    if (this != &that) [[likely]]
    {
        std::swap(m_buffer, that.m_buffer);
        std::swap(m_memory, that.m_memory);
        std::swap(m_data, that.m_data);
        std::swap(m_writeType, that.m_writeType);
    }
}

void VulkanBuffer::updateData(void* srcData, VkDeviceSize size, VkDeviceSize offset)
{
    if (!m_buffer)
        HUAN_CORE_BREAK(
        "VulkanBuffer::writeData: m_buffer is nullptr, can't write in, please invoke create function first. ");

    if (m_writeType == WriteType::Dynamic)
    {
        // Map it
        if (m_data == nullptr)
            m_data = deviceHandle.mapMemory(m_memory, offset, size);

        if (srcData)
            memcpy((char*)m_data + offset, (char*)srcData + offset, size);
    }
    else if (m_writeType == WriteType::Static)
    {
        // Just copy it, we should create a staging buffer
        auto stagingBuffer = VulkanBuffer::create(size, vk::BufferUsageFlagBits::eTransferSrc,
                                                  vk::MemoryPropertyFlagBits::eHostVisible |
                                                  vk::MemoryPropertyFlagBits::eHostCoherent);
        stagingBuffer->updateData(srcData, size, offset);
        copyFrom(stagingBuffer->m_buffer, size);
        stagingBuffer.reset();
    }
}

Scope<VulkanBuffer> VulkanBuffer::createByStagingBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                        vk::MemoryPropertyFlags memoryProperties, void* srcData)
{

    // 使用具备类实现私有构造函数
    struct EnableCreateScope : public VulkanBuffer
    {
        explicit EnableCreateScope(VulkanBuffer& that) : VulkanBuffer(that)
        {
        }
    };

    vk::Device& device = HelloTriangleApplication::getInstance()->device;
    
    VulkanBuffer vulkanBuffer;
    vulkanBuffer.m_writeType = WriteType::Static;

    vk::BufferCreateInfo bufferInfo;
    bufferInfo.setSize(size)
              .setUsage(usage)
              .setSharingMode(vk::SharingMode::eExclusive);

    vulkanBuffer.m_buffer = device.createBuffer(bufferInfo);
    if (!vulkanBuffer.m_buffer)
        HUAN_CORE_BREAK("Failed to create vertex buffer");

    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(vulkanBuffer.m_buffer);
    // 申请内存信息需要指定内存大小和内存类型索引
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setAllocationSize(memoryRequirements.size)
             .setMemoryTypeIndex(findRequiredMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryProperties));

    vulkanBuffer.m_memory = device.allocateMemory(allocInfo);
    if (!vulkanBuffer.m_memory)
        HUAN_CORE_BREAK("Failed to allocate memory for vertices! ")

    device.bindBufferMemory(vulkanBuffer.m_buffer, vulkanBuffer.m_memory, 0);
    
    auto stagingBuffer = createNormal(size, vk::BufferUsageFlagBits::eTransferSrc,
                                      vk::MemoryPropertyFlagBits::eHostVisible |
                                      vk::MemoryPropertyFlagBits::eHostCoherent, srcData);
    vulkanBuffer.copyFrom(stagingBuffer->m_buffer, size);

    stagingBuffer.reset();

    return createScope<EnableCreateScope>(vulkanBuffer);

}

Scope<VulkanBuffer> VulkanBuffer::createNormal(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                               vk::MemoryPropertyFlags memoryProperties, void* srcData)
{
    // 使用具备类实现私有构造函数
    struct EnableCreateScope : public VulkanBuffer
    {
        explicit EnableCreateScope(VulkanBuffer& that) : VulkanBuffer(that)
        {
        }
    };

    vk::Device& device = HelloTriangleApplication::getInstance()->device;

    VulkanBuffer vulkanBuffer;
    vulkanBuffer.m_writeType = WriteType::Dynamic;

    vk::BufferCreateInfo bufferInfo;
    bufferInfo.setSize(size)
              .setUsage(usage)
              .setSharingMode(vk::SharingMode::eExclusive);

    vulkanBuffer.m_buffer = device.createBuffer(bufferInfo);
    if (!vulkanBuffer.m_buffer)
        HUAN_CORE_BREAK("Failed to create vertex buffer");

    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(vulkanBuffer.m_buffer);
    // 申请内存信息需要指定内存大小和内存类型索引
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setAllocationSize(memoryRequirements.size)
             .setMemoryTypeIndex(findRequiredMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryProperties));

    vulkanBuffer.m_memory = device.allocateMemory(allocInfo);
    if (!vulkanBuffer.m_memory)
        HUAN_CORE_BREAK("Failed to allocate memory for vertices! ")

    device.bindBufferMemory(vulkanBuffer.m_buffer, vulkanBuffer.m_memory, 0);

    vulkanBuffer.updateData(srcData, size);

    return createScope<EnableCreateScope>(vulkanBuffer);

}

Scope<VulkanBuffer> VulkanBuffer::create(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                         vk::MemoryPropertyFlags memoryProperties, void* srcData)
{

    if (memoryProperties & vk::MemoryPropertyFlagBits::eHostVisible)
    {
        return createNormal(size, usage, memoryProperties, srcData);
    }
    else if (memoryProperties & vk::MemoryPropertyFlagBits::eDeviceLocal)
    {
        return createByStagingBuffer(size, usage, memoryProperties, srcData);
    }

    HUAN_CORE_BREAK("VulkanBuffer::create: Unknown memory properties! ");
    return nullptr;
}

/**
 * 获取满足类型筛选(typeFilter)和目标内存属性properties的内存类型索引
 * @param typeFilter 
 * @param properties 
 * @return 
 */
uint32_t VulkanBuffer::findRequiredMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    auto& physicalDevice = HelloTriangleApplication::getInstance()->physicalDevice;

    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (i << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    HUAN_CORE_BREAK("Failed to find suitable memory type! ")
    return 0;
}

}