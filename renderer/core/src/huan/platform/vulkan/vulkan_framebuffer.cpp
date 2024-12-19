#include "huan/platform/vulkan/vulkan_framebuffer.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include "huan/platform/vulkan/vulkan_swapchain.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{

void VulkanFramebuffer::init()
{
    VulkanSwapChain& swap_chain = VulkanContext::get_instance().get_vk_swapchain();
    VkImageView attachments[] = {swap_chain.m_image_views[0]};
    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = VulkanContext::get_instance().get_vk_render_pass().m_render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = swap_chain.m_extent.width;
    framebuffer_info.height = swap_chain.m_extent.height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(VulkanContext::get_instance().get_vk_device().m_device, &framebuffer_info, nullptr,
                            &m_framebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer!");
    }
}
void VulkanFramebuffer::cleanup()
{
    if (m_framebuffer != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(VulkanContext::get_instance().get_vk_device().m_device, m_framebuffer, nullptr);
        m_framebuffer = VK_NULL_HANDLE;
    }
}

void VulkanFramebufferSet::init()
{
    m_framebuffers.resize(VulkanContext::get_instance().get_vk_swapchain().m_image_views.size());
    for (auto& framebuffer : m_framebuffers)
    {
        framebuffer.init();
    }
}

void VulkanFramebufferSet::cleanup()
{
    for (auto& framebuffer : m_framebuffers)
    {
        framebuffer.cleanup();
    }
}
VulkanFramebuffer& VulkanFramebufferSet::get_framebuffer(uint32_t index)
{
    return m_framebuffers[index];
}

}; // namespace huan_renderer
