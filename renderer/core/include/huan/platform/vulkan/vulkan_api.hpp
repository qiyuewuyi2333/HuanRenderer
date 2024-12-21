#pragma once

#include "huan/platform/vulkan/vulkan_command_buffer.hpp"
#include "huan/platform/vulkan/vulkan_device.hpp"
#include "huan/platform/vulkan/vulkan_swapchain.hpp"
#include <cstdint>
namespace huan_renderer
{

class VulkanAPI
{
  public:
    VulkanAPI();
    ~VulkanAPI() = default;

    void init();
    void draw();

  private:
    VulkanDevice& device;
    VulkanSwapChain& swap_chain;
    VulkanCommandBuffer& command_buffer;
    std::vector<VkSemaphore>& image_available_semaphores;
    std::vector<VkSemaphore>& render_finished_semaphores;
    std::vector<VkFence>& in_flight_fences;

    uint32_t m_cur_frame_index = 0;
    const uint32_t MAX_FRAMES_IN_FLIGHT;
};
} // namespace huan_renderer
