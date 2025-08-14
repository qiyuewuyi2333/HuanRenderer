//
// Created by TU on 2025/8/10.
//

#pragma once
#include "huan/common.hpp"

#include <vulkan/vulkan.hpp>

namespace huan::runtime::vulkan
{
// TODO: jk
class VulkanManager
{
public:
    void Init();

private:
    vk::Instance m_instance{};
    vk::PhysicalDevice m_physicalDevice{};
    vk::Device m_device{};

};
}