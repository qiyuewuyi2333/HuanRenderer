#include "huan/core/renderer.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{
void Renderer::init()
{
    // Renderer just borrow the context from Vulkan
    m_context = &VulkanContext::get_instance();
    m_context->init();

    create_sync_objects();
}
void Renderer::create_sync_objects()
{
    VkSemaphoreCreateInfo semaphore_info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (vkCreateSemaphore(m_context->get_vk_device().m_device, &semaphore_info, nullptr,
                          &m_image_available_semaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image available semaphore!");
    }

    if (vkCreateSemaphore(m_context->get_vk_device().m_device, &semaphore_info, nullptr,
                          &m_render_finished_semaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render finished semaphore!");
    }
    VkFenceCreateInfo fence_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    if (vkCreateFence(m_context->get_vk_device().m_device, &fence_info, nullptr, &m_in_flight_fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create in-flight fence!");
    }
}
void Renderer::shutdown()
{
    vkDestroySemaphore(m_context->get_vk_device().m_device, m_image_available_semaphore, nullptr);
    vkDestroySemaphore(m_context->get_vk_device().m_device, m_render_finished_semaphore, nullptr);
    vkDestroyFence(m_context->get_vk_device().m_device, m_in_flight_fence, nullptr);
    m_context->cleanup();
}

void Renderer::draw()
{
    // 等待当前帧的栅栏信号，确保上一次的绘制操作已完成
    vkWaitForFences(m_context->get_vk_device().m_device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_context->get_vk_device().m_device, 1, &m_in_flight_fence);

    uint32_t image_index = 0;
    VkResult acquire_result =
        vkAcquireNextImageKHR(m_context->get_vk_device().m_device, m_context->get_vk_swapchain().m_swap_chain,
                              UINT64_MAX, m_image_available_semaphore, VK_NULL_HANDLE, &image_index);

    if (acquire_result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to acquire next swapchain image!");
    }

    // 重置命令缓冲区以准备新的绘制命令
    vkResetCommandBuffer(m_context->get_vk_command_buffer().m_command_buffer, 0);
    m_context->get_vk_command_buffer().record_command_buffer(image_index);

    // 提交渲染命令
    VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};

    // 等待图像可用信号量
    VkSemaphore wait_semaphores[] = {m_image_available_semaphore};
    VkSemaphore signal_semaphores[] = {m_render_finished_semaphore};

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    // 提交命令缓冲区
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_context->get_vk_command_buffer().m_command_buffer;

    // 渲染完成后触发的信号量
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    // 提交渲染命令到图形队列
    if (vkQueueSubmit(m_context->get_vk_device().m_graphics_queue, 1, &submit_info, m_in_flight_fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    // 等待渲染完成信号量后执行交换链呈现操作
    VkSwapchainKHR swap_chains[] = {m_context->get_vk_swapchain().m_swap_chain};
    VkPresentInfoKHR present_info{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores; // 等待渲染完成的信号量
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;

    // 将图像呈现到屏幕
    VkResult result = vkQueuePresentKHR(m_context->get_vk_device().m_present_queue, &present_info);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    std::cout << "Image index: " << image_index << ", Present result: " << result << std::endl;
}

} // namespace huan_renderer
