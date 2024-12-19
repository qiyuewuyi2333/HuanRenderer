#pragma once

#include <vulkan/vulkan_core.h>
namespace huan_renderer
{
class VulkanCommandPool
{
  public:
    VulkanCommandPool() = default;
    ~VulkanCommandPool() = default;

    void init();
    void cleanup();

  public:
    VkCommandPool m_command_pool = VK_NULL_HANDLE;
};
} // namespace huan_renderer
