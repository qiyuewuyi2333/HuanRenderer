//
// Created by 86156 on 5/12/2025.
//

#include <huan/backend/resource/sampler.hpp>

#include "huan/log/Log.hpp"

namespace huan::vulkan
{

Sampler::Sampler(vk::Device& deviceHandle, const vk::SamplerCreateInfo& info)
    : VulkanResource(deviceHandle, VK_NULL_HANDLE)
{
    getHandle() = deviceHandle.createSampler(info);
    if (getHandle() == VK_NULL_HANDLE)
        HUAN_CORE_ERROR("Renderer: Failed to create sampler. ")
}

Sampler::Sampler(Sampler&& that)
    : VulkanResource(std::move(that))
{
}

Sampler::~Sampler()
{
    getDeviceHandle().destroySampler(getHandle());
}
}