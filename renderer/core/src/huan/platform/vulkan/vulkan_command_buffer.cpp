#include "huan/platform/vulkan/vulkan_command_buffer.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{

void VulkanCommandBuffer::init()
{
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = VulkanContext::get_instance().get_vk_command_pool().m_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32_t)(VulkanContext::get_instance().get_app_info()->max_frames_in_flight),

    };
    m_command_buffer.resize(VulkanContext::get_instance().get_app_info()->max_frames_in_flight);
    if (vkAllocateCommandBuffers(VulkanContext::get_instance().get_vk_device().m_device, &allocInfo,
                                 m_command_buffer.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
void VulkanCommandBuffer::cleanup()
{
    vkFreeCommandBuffers(VulkanContext::get_instance().get_vk_device().m_device,
                         VulkanContext::get_instance().get_vk_command_pool().m_command_pool,
                         (uint32_t)(VulkanContext::get_instance().get_app_info()->max_frames_in_flight),
                         m_command_buffer.data());
}

void VulkanCommandBuffer::record_command_buffer(uint32_t current_frame, uint32_t image_index)
{
    VkCommandBufferBeginInfo begin_info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    if (vkBeginCommandBuffer(m_command_buffer[current_frame], &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    // Record commands here
    // Example command: Begin render pass
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = VulkanContext::get_instance().get_vk_render_pass().m_render_pass;
    render_pass_info.framebuffer =
        VulkanContext::get_instance().get_vk_frambuffers().get_framebuffer(image_index).m_framebuffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = VulkanContext::get_instance().get_vk_swapchain().m_extent;

    VkClearValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(m_command_buffer[current_frame], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    // Example command: Bind pipeline
    vkCmdBindPipeline(m_command_buffer[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      VulkanContext::get_instance().get_vk_pipeline().m_graphics_pipeline);
    VkViewport viewport{.x = 0.0f,
                        .y = 0.0f,
                        .width = static_cast<float>(VulkanContext::get_instance().get_vk_swapchain().m_extent.width),
                        .height = static_cast<float>(VulkanContext::get_instance().get_vk_swapchain().m_extent.height),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f};
    vkCmdSetViewport(m_command_buffer[current_frame], 0, 1, &viewport);

    VkRect2D scissor{.offset = {0, 0}, .extent = VulkanContext::get_instance().get_vk_swapchain().m_extent};
    vkCmdSetScissor(m_command_buffer[current_frame], 0, 1, &scissor);

    // Example command: Draw
    vkCmdDraw(m_command_buffer[current_frame], 3, 1, 0, 0);

    // Example command: End render pass
    vkCmdEndRenderPass(m_command_buffer[current_frame]);

    if (vkEndCommandBuffer(m_command_buffer[current_frame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

} // namespace huan_renderer
