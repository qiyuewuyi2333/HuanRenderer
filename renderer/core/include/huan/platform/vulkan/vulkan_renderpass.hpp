#pragma once

#include <vulkan/vulkan_core.h>
namespace huan_renderer
{

class VulkanRenderPass
{
  public:
    void init();
    void cleanup();

  public:
    VkRenderPass m_render_pass = VK_NULL_HANDLE;
};

} // namespace huan_renderer
