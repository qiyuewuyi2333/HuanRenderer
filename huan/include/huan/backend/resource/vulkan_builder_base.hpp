#pragma once

#include <string>
#include <vector>
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "huan/common.hpp"

namespace huan::runtime::vulkan
{
template <class BuilderType, class CreateInfoType>
class BuilderBase
{
public:
    [[nodiscard]] const VmaAllocationCreateInfo& getAllocationCreateInfo() const;
    VmaAllocator getAllocator() const;
    const CreateInfoType& getCreateInfo() const;
    [[nodiscard]] const std::string& getDebugName() const;
    BuilderType& setDebugName(const std::string& debugName);
    BuilderType& setImplicitSharingMode();
    BuilderType& setMemoryTypeBits(uint32_t typeBits);
    BuilderType& setQueueFamilies(uint32_t count, const uint32_t* familyIndices);
    BuilderType& setQueueFamilies(const std::vector<uint32_t>& queueFamilies);
    BuilderType& setSharingMode(vk::SharingMode sharingMode);
    BuilderType& setVmaFlags(VmaAllocationCreateFlags flags);
    BuilderType& setVmaPool(VmaPool pool);
    BuilderType& setVmaPreferredFlags(vk::MemoryPropertyFlags flags);
    BuilderType& setVmaRequiredFlags(vk::MemoryPropertyFlags flags);
    BuilderType& setVmaUsage(VmaMemoryUsage usage);

protected:
    HUAN_NO_COPY(BuilderBase)
    explicit BuilderBase(VmaAllocator allocator, const CreateInfoType& createInfo);
    CreateInfoType& getCreateInfo();

    VmaAllocator m_allocator{};
    VmaAllocationCreateInfo m_allocationCreateInfo = {};
    CreateInfoType m_createInfo{};
#ifdef HUAN_DEBUG
    std::string m_debugName = {};
#endif
};

template <class BuilderType, class CreateInfoType>
const VmaAllocationCreateInfo& BuilderBase<BuilderType, CreateInfoType>::getAllocationCreateInfo() const
{
    return m_allocationCreateInfo;
}

template <class BuilderType, class CreateInfoType>
VmaAllocator BuilderBase<BuilderType, CreateInfoType>::getAllocator() const
{
    return m_allocator;
}

template <class BuilderType, class CreateInfoType>
const CreateInfoType& BuilderBase<BuilderType, CreateInfoType>::getCreateInfo() const
{
    return m_createInfo;
}

template <class BuilderType, class CreateInfoType>
const std::string& BuilderBase<BuilderType, CreateInfoType>::getDebugName() const
{
#ifdef HUAN_DEBUG
    return m_debugName;
#elif
    return {};
#endif
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setDebugName(const std::string& debugName)
{
#ifdef HUAN_DEBUG
    m_debugName = debugName;
#endif
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setImplicitSharingMode()
{
    m_createInfo.sharingMode = vk::SharingMode::eExclusive;
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setMemoryTypeBits(uint32_t typeBits)
{
    m_allocationCreateInfo.memoryTypeBits = typeBits;
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setQueueFamilies(uint32_t count, const uint32_t* familyIndices)
{
    m_createInfo.queueFamilyIndexCount = count;
    m_createInfo.pQueueFamilyIndices = familyIndices;
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setQueueFamilies(const std::vector<uint32_t>& queueFamilies)
{
    return setQueueFamilies(queueFamilies.size(), queueFamilies.data());
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setSharingMode(vk::SharingMode sharingMode)
{
    m_createInfo.sharingMode = sharingMode;
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setVmaFlags(VmaAllocationCreateFlags flags)
{
    m_allocationCreateInfo.flags = flags;
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setVmaPool(VmaPool pool)
{
    m_allocationCreateInfo.pool = pool;
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setVmaPreferredFlags(vk::MemoryPropertyFlags flags)
{
    m_allocationCreateInfo.preferredFlags = static_cast<VkMemoryPropertyFlags>(flags);
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setVmaRequiredFlags(vk::MemoryPropertyFlags flags)
{
    m_allocationCreateInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(flags);
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderType& BuilderBase<BuilderType, CreateInfoType>::setVmaUsage(VmaMemoryUsage usage)
{
    m_allocationCreateInfo.usage = usage;
    return *static_cast<BuilderType*>(this);
}

template <class BuilderType, class CreateInfoType>
BuilderBase<BuilderType, CreateInfoType>::BuilderBase(VmaAllocator allocator, const CreateInfoType& createInfo)
    : m_allocator(allocator), m_createInfo(createInfo)
{
    m_allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
}

template <class BuilderType, class CreateInfoType>
CreateInfoType& BuilderBase<BuilderType, CreateInfoType>::getCreateInfo()
{
    return m_createInfo;
}
} // namespace huan::vulkan