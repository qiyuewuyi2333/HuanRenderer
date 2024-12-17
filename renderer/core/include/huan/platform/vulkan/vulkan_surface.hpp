#pragma once

#include <vulkan/vulkan_core.h>
namespace huan_renderer
{
class VulkanSurface
{
  public:
    VulkanSurface() = default;
    ~VulkanSurface() = default;

    void init();
    void cleanup();

  public:
    VkSurfaceKHR m_surface;
};

} // namespace huan_renderer
