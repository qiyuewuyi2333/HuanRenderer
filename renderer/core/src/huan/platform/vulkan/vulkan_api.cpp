#pragma once
#include "huan/platform/vulkan/vulkan_api.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include <cstdint>
#include <stdexcept>

namespace huan_renderer
{
VulkanAPI::VulkanAPI()
    : MAX_FRAMES_IN_FLIGHT(VulkanContext::get_instance().get_app_info()->max_frames_in_flight),
      device(VulkanContext::get_instance().get_vk_device()),
      swap_chain(VulkanContext::get_instance().get_vk_swapchain()),
      command_buffer(VulkanContext::get_instance().get_vk_command_buffer()),
      image_available_semaphores(VulkanContext::get_instance().get_image_available_semaphores()),
      render_finished_semaphores(VulkanContext::get_instance().get_render_finished_semaphores()),
      in_flight_fences(VulkanContext::get_instance().get_in_flight_fences())
{
}
void VulkanAPI::init()
{
}
void VulkanAPI::draw()
{
    // 等待当前帧的栅栏信号，确保上一次的绘制操作已完成
    vkWaitForFences(device.m_device, 1, &in_flight_fences[m_cur_frame_index], VK_TRUE, UINT64_MAX);
    vkResetFences(device.m_device, 1, &in_flight_fences[m_cur_frame_index]);

    uint32_t image_index = 0;
    VkResult acquire_result =
        vkAcquireNextImageKHR(device.m_device, swap_chain.m_swap_chain, UINT64_MAX,
                              image_available_semaphores[m_cur_frame_index], VK_NULL_HANDLE, &image_index);

    if (acquire_result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to acquire next swapchain image!");
    }

    // 重置命令缓冲区以准备新的绘制命令
    vkResetCommandBuffer(command_buffer.m_command_buffer[m_cur_frame_index], 0);
    command_buffer.record_command_buffer(m_cur_frame_index, image_index);

    // 提交渲染命令
    VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};

    // 等待图像可用信号量
    VkSemaphore wait_semaphores[] = {image_available_semaphores[m_cur_frame_index]};
    VkSemaphore signal_semaphores[] = {render_finished_semaphores[m_cur_frame_index]};

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    // 提交命令缓冲区
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer.m_command_buffer[m_cur_frame_index];

    // 渲染完成后触发的信号量
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    // 提交渲染命令到图形队列
    if (vkQueueSubmit(device.m_graphics_queue, 1, &submit_info, in_flight_fences[m_cur_frame_index]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    // 等待渲染完成信号量后执行交换链呈现操作
    VkSwapchainKHR swap_chains[] = {swap_chain.m_swap_chain};
    VkPresentInfoKHR present_info{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores; // 等待渲染完成的信号量
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;

    // 将图像呈现到屏幕
    VkResult result = vkQueuePresentKHR(device.m_present_queue, &present_info);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    m_cur_frame_index = (m_cur_frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

} // namespace huan_renderer
