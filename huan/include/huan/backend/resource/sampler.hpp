//
// Created by 86156 on 5/12/2025.
//
#pragma once

#include "vulkan_resource.hpp"

namespace huan::runtime::vulkan
{
class Sampler final : public VulkanResource<vk::Sampler>
{
  public:
    Sampler(vk::Device& deviceHandle, const vk::SamplerCreateInfo& info);
    HUAN_NO_COPY(Sampler)
    Sampler(Sampler&& that) noexcept = default;
    Sampler& operator=(Sampler&& that) = delete;
    ~Sampler() override;
};
} // namespace huan::vulkan