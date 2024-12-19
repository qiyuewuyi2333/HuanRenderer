#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
namespace huan_renderer
{
class VulkanCommandBuffer
{
  public:
    VulkanCommandBuffer() = default;
    ~VulkanCommandBuffer() = default;

    void init();
    void cleanup();

    void record_command_buffer(uint32_t image_index);

  public:
    VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
};

} // namespace huan_renderer
