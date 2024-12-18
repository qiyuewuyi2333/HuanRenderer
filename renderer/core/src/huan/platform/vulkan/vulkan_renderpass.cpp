#include "huan/platform/vulkan/vulkan_renderpass.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{

void VulkanRenderPass::init()
{
    VkAttachmentDescription color_attachment{.format = VulkanContext::get_instance().get_vk_swapchain().m_format.format,
                                             .samples = VK_SAMPLE_COUNT_1_BIT,
                                             .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                             .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                             .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                             .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                             .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                             .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    VkAttachmentReference color_attachment_ref{.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 .colorAttachmentCount = 1,
                                 .pColorAttachments = &color_attachment_ref};

    VkRenderPassCreateInfo render_pass_info{.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                            .attachmentCount = 1,
                                            .pAttachments = &color_attachment,
                                            .subpassCount = 1,
                                            .pSubpasses = &subpass};

    if (vkCreateRenderPass(VulkanContext::get_instance().get_vk_device().m_device, &render_pass_info, nullptr,
                           &m_render_pass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}
void VulkanRenderPass::cleanup()
{
    if (m_render_pass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(VulkanContext::get_instance().get_vk_device().m_device, m_render_pass, nullptr);
        m_render_pass = VK_NULL_HANDLE;
    }
}

} // namespace huan_renderer
