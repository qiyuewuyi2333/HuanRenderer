//
// Created by 86156 on 5/12/2025.
//

#include <huan/backend/resource/sampler.hpp>

#include "huan/log/Log.hpp"

namespace huan::runtime::vulkan
{

Sampler::Sampler(vk::Device& deviceHandle, const vk::SamplerCreateInfo& info)
    : VulkanResource(deviceHandle, VK_NULL_HANDLE)
{
    getHandle() = deviceHandle.createSampler(info);
    if (getHandle() == VK_NULL_HANDLE)
        HUAN_CORE_ERROR("Renderer: Failed to create sampler. ")
}

Sampler::Sampler(Sampler&& that) noexcept : ParentType(std::move(that))
{
}
Sampler& Sampler::operator=(Sampler&& that) noexcept
{
    if (this != &that)
    {
        getDeviceHandle().destroySampler(getHandle());
        getHandle() = that.getHandle();
        that.getHandle() = VK_NULL_HANDLE;
    }
    return *this;
}
Sampler::~Sampler()
{
    getDeviceHandle().destroySampler(getHandle());
}
} // namespace huan::runtime::vulkan