//
// Created by 86156 on 5/12/2025.
//
#pragma once

#include <vulkan/vulkan.hpp>

#include "huan/common.hpp"

namespace huan::vulkan
{
template <class ResourceType>
class VulkanResource
{
public:
    explicit VulkanResource(vk::Device& deviceHandle, ResourceType handle = nullptr);
    HUAN_NO_COPY(VulkanResource)
    VulkanResource(VulkanResource&& that) noexcept;
    VulkanResource& operator=(VulkanResource&& that) noexcept;
    virtual ~VulkanResource() = default;

    [[nodiscard]] std::string_view getDebugName() const;
    ResourceType& getHandle();
    vk::Device& getDeviceHandle();
    void setHandle(ResourceType handle);
    [[nodiscard]] const ResourceType& getHandle() const;
    [[nodiscard]] uint64_t getHandleU64() const;

private:
#ifdef HUAN_DEBUG
    std::string m_debugName;
#endif
    vk::Device& m_deviceHandle;
    ResourceType m_resourceHandle;
};

template <class ResourceType>
VulkanResource<ResourceType>::VulkanResource(vk::Device& deviceHandle, ResourceType handle)
    : m_deviceHandle(deviceHandle), m_resourceHandle(handle)
{
}

template <class ResourceType>
VulkanResource<ResourceType>::VulkanResource(VulkanResource&& that) noexcept
    : m_deviceHandle(that.m_deviceHandle), m_resourceHandle(that.m_resourceHandle)
#ifdef HUAN_DEBUG
    , m_debugName(std::move(that.m_debugName))
#endif
{
}

template <class ResourceType>
VulkanResource<ResourceType>& VulkanResource<ResourceType>::operator=(VulkanResource&& that) noexcept
{
    m_resourceHandle = std::exchange(that.m_resourceHandle, {});
    m_deviceHandle = std::exchange(that.m_deviceHandle, {});
#ifdef HUAN_DEBUG
    m_debugName = std::exchange(that.m_debugName, {});
# endif
    return *this;
}

template <class ResourceType>
std::string_view VulkanResource<ResourceType>::getDebugName() const
{
#ifdef HUAN_DEBUG
    return m_debugName;
#else
    return {};
#endif
}

template <class ResourceType>
ResourceType& VulkanResource<ResourceType>::getHandle()
{
    return m_resourceHandle;
}

template <class ResourceType>
vk::Device& VulkanResource<ResourceType>::getDeviceHandle()
{
    return m_deviceHandle;
}

template <class ResourceType>
void VulkanResource<ResourceType>::setHandle(ResourceType handle)
{
    m_resourceHandle = handle;
}

template <class ResourceType>
const ResourceType& VulkanResource<ResourceType>::getHandle() const
{
    return m_resourceHandle;
}

template <class ResourceType>
uint64_t VulkanResource<ResourceType>::getHandleU64() const
{
    // See https://github.com/KhronosGroup/Vulkan-Docs/issues/368 .
    // Dispatchable and non-dispatchable handle types are *not* necessarily binary-compatible!
    // Non-dispatchable handles _might_ be only 32-bit long. This is because, on 32-bit machines, they might be a typedef to a 32-bit pointer.
    using UintHandle = std::conditional_t<sizeof(ResourceType) == sizeof(uint32_t), uint32_t, uint64_t>;

    return static_cast<uint64_t>(*reinterpret_cast<UintHandle const *>(&m_resourceHandle));
}
}