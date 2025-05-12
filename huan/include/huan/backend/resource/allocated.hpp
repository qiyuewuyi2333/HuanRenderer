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
    VulkanAllocated() = delete;
    HUAN_NO_COPY(VulkanAllocated)
    VulkanAllocated(VulkanAllocated&& that) noexcept = default;
    VulkanAllocated& operator=(VulkanAllocated&& that) = default;

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
    

    
};
}