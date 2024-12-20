#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
namespace huan_renderer
{

class VulkanFramebuffer
{
  public:
    void init(uint32_t view_index);
    void cleanup();

  public:
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
};

class VulkanFramebufferSet
{
  public:
    VulkanFramebufferSet() = default;
    ~VulkanFramebufferSet() = default;

    void init();
    void cleanup();

    VulkanFramebuffer& get_framebuffer(uint32_t index);

  private:
    std::vector<VulkanFramebuffer> m_framebuffers;
};

} // namespace huan_renderer
