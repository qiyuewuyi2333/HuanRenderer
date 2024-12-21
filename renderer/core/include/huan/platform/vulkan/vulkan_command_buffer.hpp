#pragma once

#include <cstdint>
#include <vector>
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

    void record_command_buffer(uint32_t current_frame, uint32_t image_index);

  public:
    std::vector<VkCommandBuffer> m_command_buffer;
};

} // namespace huan_renderer
