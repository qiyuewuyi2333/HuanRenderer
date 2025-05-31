//
// Created by 86156 on 5/13/2025.
//
#pragma once
#include "vk_mem_alloc.h"
#include "vulkan_resource.hpp"
#include "vulkan_resources.hpp"
#include "huan/common.hpp"

namespace huan::vulkan
{
template <class ResourceType>
class VulkanAllocated : public VulkanResource<ResourceType>
{
  public:
    using ParentType = ::huan::vulkan::VulkanResource<ResourceType>;

  public:
    VulkanAllocated() = delete;
    HUAN_NO_COPY(VulkanAllocated)
    VulkanAllocated(VulkanAllocated&& that) noexcept;
    VulkanAllocated& operator=(VulkanAllocated&& that) = delete;

  protected:
    /**
     * @param allocInfo Used for VMA
     * @param args To create derived class. Typically a  'vk::ImageCreateInfo' or 'vk::BufferCreateInfo'
     */
    template <class... Args>
    VulkanAllocated(const VmaAllocationCreateInfo& allocInfo, Args&&... args);
    VulkanAllocated(vk::Device& deviceHandle, ResourceType handle);

  public:
    [[nodiscard]] const ResourceType& get() const;
    /**
     * Flush this memory if it is NOT `HOST_COHERENT`.
     * @param offset
     * @param size
     */
    void flush(vk::DeviceSize offset = 0, vk::DeviceSize size = vk::WholeSize);
    /**
     * Retrieve a pointer to the host visible memory as an ubyte array.
     * @return The pointer is host visible memory.
     * @note No checking for if the memory is actually mapped, so be careful to get a nullptr.
     */
    [[nodiscard]] const uint8_t* getData() const;
    /**
     * Retrieve the raw vulkan memory object.
     * @return The vulkan memory object.
     */
    [[nodiscard]] vk::DeviceMemory getDeviceMemory() const;
    /**
     * Map vulkan memory if it isn't already mapped to a host visible address.
     * Does nothing if the allocation is already mapped ( including persistently mapped).
     * @return Pointer to host visible memory.
     */
    uint8_t* map();
    /**
     * @return Mapping status.
     */
    bool mapped() const;
    /**
     * Unmap vulkan memory from the host visible address.
     * Do nothing if no mapped or persistently mapped.
     */
    void unmap();

    /**
     * Directly copy data to the host visible memory. Don't need mapping, and also don't do unmapping!
     * @param data A pointer with data, and it SHOULD have been mapped to the host visible address.
     * @param size Bytes num
     * @param offset Pos to start
     * @return
     */
    size_t updateDirectly(const uint8_t* data, size_t size, size_t offset = 0);
    /**
     * Update with mapping and unmapping for non-persistent memory. Please don't use this function if you want to update
     * one memory for many times.
     * @param data A pointer with data, and it SHOULD have been mapped to the host visible address.
     * @param size Bytes num
     * @param offset Pos to start
     * @return
     */
    size_t updateWithMapping(const uint8_t* data, size_t size, size_t offset = 0);
    /**
     * Overload to updateDirectly(const uint8_t* data, size_t size, size_t offset = 0);
     * But it allows any kind of data. And the data will be copied to the buffer as bytes.
     */
    size_t updateDirectly(const void* data, size_t size, size_t offset = 0);
    /**
     * Overload to updateMapping(const uint8_t* data, size_t size, size_t offset = 0);
     * But it allows any kind of data. And the data will be copied to the buffer as bytes.
     */
    size_t updateWithMapping(const void* data, size_t size, size_t offset = 0);

    template <class T>
    size_t updateDirectly(const vk::ArrayProxy<T>& object, size_t offset = 0);
    template <class T>
    size_t updateWithMapping(const vk::ArrayProxy<T>& object, size_t offset = 0);

  protected:
    [[nodiscard]] vk::Buffer createBuffer(const vk::BufferCreateInfo& createInfo);
    [[nodiscard]] vk::Image createImage(const vk::ImageCreateInfo& createInfo);
    void destroyBuffer(vk::Buffer buffer);
    void destroyImage(vk::Image image);
    // For derived class to call in createXXX
    virtual void postCreate(const VmaAllocationInfo& allocationInfo);
    // Clear the internal state in destroyXXX
    virtual void clear();

  private:
    VmaAllocationCreateInfo m_allocationCreateInfo = {};
    VmaAllocation m_allocation = nullptr;
    /**
     * Pointer to the allocation memory, if host visible or (persistently)mapped.
     */
    uint8_t* m_mappedData = nullptr;

    /**
     * If the allocation is HOST_COHERENT, it will be flushed automatically.
     */
    bool m_isCoherent = false;
    /**
     * If the allocation is PERSISTENT, it will be mapped automatically.
     * Not just HOST_VISIBLE, but available as a pointer to the application along the lifetime.
     * @note This is initialized at allocation time to avoid sub sequent need to call a function to fetch the allocation
     * information from the VMA, since this property won't change for the lifetime of the allocation.
     */
    bool m_isPersistent = false;
};

template <class ResourceType>
VulkanAllocated<ResourceType>::VulkanAllocated(VulkanAllocated&& that) noexcept
    : ParentType{static_cast<ParentType&&>(that)},
      m_allocationCreateInfo(std::exchange(that.m_allocationCreateInfo, {})),
      m_allocation(std::exchange(that.m_allocation, nullptr)), m_mappedData(std::exchange(that.m_mappedData, nullptr)),
      m_isCoherent(std::exchange(that.m_isCoherent, false)), m_isPersistent(std::exchange(that.m_isPersistent, false))
{
}

template <class ResourceType>
template <class... Args>
VulkanAllocated<ResourceType>::VulkanAllocated(const VmaAllocationCreateInfo& allocInfo, Args&&... args)
    : ParentType{std::forward<Args>(args)...}, m_allocationCreateInfo(allocInfo)
{
}

template <class ResourceType>
VulkanAllocated<ResourceType>::VulkanAllocated(vk::Device& deviceHandle, ResourceType handle)
    : ParentType(deviceHandle, handle)
{
}

template <class ResourceType>
const ResourceType& VulkanAllocated<ResourceType>::get() const
{
    return ParentType::getHandle();
}

template <class ResourceType>
void VulkanAllocated<ResourceType>::flush(vk::DeviceSize offset, vk::DeviceSize size)
{
    if (!m_isCoherent)
    {
        vmaFlushAllocation(ResourceSystem::getInstance()->allocatorHandle, m_allocation, offset, size);
    }
}

template <class ResourceType>
const uint8_t* VulkanAllocated<ResourceType>::getData() const
{
    return m_mappedData;
}

template <class ResourceType>
vk::DeviceMemory VulkanAllocated<ResourceType>::getDeviceMemory() const
{
    VmaAllocationInfo allocInfo;
    // Get alloc info
    vmaGetAllocationInfo(ResourceSystem::getInstance()->allocatorHandle, m_allocation, &allocInfo);

    return allocInfo.deviceMemory;
}

template <class ResourceType>
uint8_t* VulkanAllocated<ResourceType>::map()
{
    if (!m_isPersistent && !mapped())
    {
        VK_CHECK(vmaMapMemory(ResourceSystem::getInstance()->allocatorHandle, m_allocation,
                              reinterpret_cast<void**>(&m_mappedData)))
    }
    return m_mappedData;
}

template <class ResourceType>
bool VulkanAllocated<ResourceType>::mapped() const
{
    return m_mappedData != nullptr;
}

template <class ResourceType>
void VulkanAllocated<ResourceType>::unmap()
{
    if (!m_isPersistent && mapped())
    {
        vmaUnmapMemory(ResourceSystem::getInstance()->allocatorHandle, m_allocation);
        m_mappedData = nullptr;
    }
}

template <class ResourceType>
size_t VulkanAllocated<ResourceType>::updateDirectly(const uint8_t* data, size_t size, size_t offset)
{
    std::copy_n(data, size, m_mappedData + offset);
    flush(offset, size);

    return size;
}

template <class ResourceType>
size_t VulkanAllocated<ResourceType>::updateWithMapping(const uint8_t* data, size_t size, size_t offset)
{
    if (m_isPersistent)
    {
        std::copy_n(data, size, m_mappedData + offset);
        flush(offset, size);
    }
    else
    {
        map();
        std::copy_n(data, size, m_mappedData + offset);
        flush(offset, size);
        unmap();
    }
}

template <class ResourceType>
size_t VulkanAllocated<ResourceType>::updateDirectly(const void* data, size_t size, size_t offset)
{
    return updateDirectly(static_cast<const uint8_t*>(data), size, offset);
}

template <class ResourceType>
size_t VulkanAllocated<ResourceType>::updateWithMapping(const void* data, size_t size, size_t offset)
{
    return updateWithMapping(static_cast<const uint8_t*>(data), size, offset);
}

template <class ResourceType>
template <class T>
size_t VulkanAllocated<ResourceType>::updateDirectly(const vk::ArrayProxy<T>& object, size_t offset)
{
    return updateDirectly(static_cast<const uint8_t*>(object.data()), object.size() * sizeof(T), offset);
}

template <class ResourceType>
template <class T>
size_t VulkanAllocated<ResourceType>::updateWithMapping(const vk::ArrayProxy<T>& object, size_t offset)
{
    return updateWithMapping(static_cast<const uint8_t*>(object.data()), object.size() * sizeof(T), offset);
}
template <class ResourceType>
vk::Buffer VulkanAllocated<ResourceType>::createBuffer(const vk::BufferCreateInfo& createInfo)
{
    vk::Buffer buffer = VK_NULL_HANDLE;
    VmaAllocationInfo allocationInfo{};
    auto resSys = ResourceSystem::getInstance();
    auto res =
        vmaCreateBuffer(resSys->allocatorHandle, reinterpret_cast<const VkBufferCreateInfo*>(&createInfo),
                        &m_allocationCreateInfo, reinterpret_cast<VkBuffer*>(&buffer), &m_allocation, &allocationInfo);
    if (res != VK_SUCCESS)
        HUAN_CORE_BREAK("Failed to create buffer");

    postCreate(allocationInfo);
    return buffer;
}

template <class ResourceType>
vk::Image VulkanAllocated<ResourceType>::createImage(const vk::ImageCreateInfo& createInfo)
{
    HUAN_CORE_ASSERT(0 < createInfo.mipLevels, "Mip levels must be greater than 0");
    HUAN_CORE_ASSERT(0 < createInfo.arrayLayers, "Array layers must be greater than 0");
    HUAN_CORE_ASSERT(createInfo.usage, "Usage must be greater than 0");

    vk::Image image = VK_NULL_HANDLE;
    VmaAllocationInfo allocationInfo{};
    const auto resSys = ResourceSystem::getInstance();
    const auto res =
        vmaCreateImage(resSys->allocatorHandle, reinterpret_cast<const VkImageCreateInfo*>(&createInfo),
                       &m_allocationCreateInfo, reinterpret_cast<VkImage*>(&image), &m_allocation, &allocationInfo);
    if (res != VK_SUCCESS)
        HUAN_CORE_BREAK("Failed to create image");

    postCreate(allocationInfo);
    return image;
}
template <class ResourceType>
void VulkanAllocated<ResourceType>::destroyBuffer(vk::Buffer buffer)
{
    if (buffer != VK_NULL_HANDLE && m_allocation != nullptr)
    {
        unmap();
        vmaDestroyBuffer(ResourceSystem::getInstance()->allocatorHandle, static_cast<VkBuffer>(buffer), m_allocation);
        clear();
    }
}
template <class ResourceType>
void VulkanAllocated<ResourceType>::destroyImage(vk::Image image)
{
    if (image != VK_NULL_HANDLE && m_allocation != nullptr)
    {
        unmap();
        vmaDestroyImage(ResourceSystem::getInstance()->allocatorHandle, static_cast<VkImage>(image), m_allocation);
        clear();
    }
}
template <class ResourceType>
void VulkanAllocated<ResourceType>::postCreate(const VmaAllocationInfo& allocationInfo)
{
}
template <class ResourceType>
void VulkanAllocated<ResourceType>::clear()
{
    m_mappedData = nullptr;
    m_isPersistent = false;
    m_allocationCreateInfo = {};
}
} // namespace huan::vulkan