//
// Created by 86156 on 5/13/2025.
//
#pragma once
#include "vk_mem_alloc.h"
#include "vulkan_resource.hpp"

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
     * Update with mapping and unmapping for non-persistent memory. Please don't use this function if you want to update one memory for many times.
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

private:
    VmaAllocationCreateInfo m_allocationCreateInfo = {};
    VmaAllocation m_allocation = nullptr;
    /**
     * Pointer to the allocation memory, if host visible or (persistently)mapped.
     */
    uint8_t* m_mappedData = nullptr;
    bool m_isCoherent = false;
    bool m_isPersistent = false;
};

template <class ResourceType>
VulkanAllocated<ResourceType>::VulkanAllocated(VulkanAllocated&& that) noexcept
    : ParentType{static_cast<ParentType&&>(that)},
      m_allocationCreateInfo(std::exchange(that.m_allocationCreateInfo, {})),
      m_allocation(std::exchange(that.m_allocation, nullptr)),
      m_mappedData(std::exchange(that.m_mappedData, nullptr)),
      m_isCoherent(std::exchange(that.m_isCoherent, false)),
      m_isPersistent(std::exchange(that.m_isPersistent, false))
{

}

template <class ResourceType>
template <class... Args>
VulkanAllocated<ResourceType>::VulkanAllocated(const VmaAllocationCreateInfo& allocInfo, Args&&... args)
    : ParentType{std::forward<Args>(args)...},
      m_allocationCreateInfo(allocInfo)
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
}

template <class ResourceType>
const uint8_t* VulkanAllocated<ResourceType>::getData() const
{
}

template <class ResourceType>
vk::DeviceMemory VulkanAllocated<ResourceType>::getDeviceMemory() const
{
}

template <class ResourceType>
uint8_t* VulkanAllocated<ResourceType>::map()
{
}

template <class ResourceType>
bool VulkanAllocated<ResourceType>::mapped() const
{
}

template <class ResourceType>
void VulkanAllocated<ResourceType>::unmap()
{
}

template <class ResourceType>
size_t VulkanAllocated<ResourceType>::updateDirectly(const uint8_t* data, size_t size, size_t offset)
{
}

template <class ResourceType>
size_t VulkanAllocated<ResourceType>::updateWithMapping(const uint8_t* data, size_t size, size_t offset)
{
}

template <class ResourceType>
size_t VulkanAllocated<ResourceType>::updateDirectly(const void* data, size_t size, size_t offset)
{
}

template <class ResourceType>
size_t VulkanAllocated<ResourceType>::updateWithMapping(const void* data, size_t size, size_t offset)
{
}

template <class ResourceType>
template <class T>
size_t VulkanAllocated<ResourceType>::updateDirectly(const vk::ArrayProxy<T>& object, size_t offset)
{
}

template <class ResourceType>
template <class T>
size_t VulkanAllocated<ResourceType>::updateWithMapping(const vk::ArrayProxy<T>& object, size_t offset)
{
}
}