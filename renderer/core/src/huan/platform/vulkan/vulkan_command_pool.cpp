#include "huan/platform/vulkan/vulkan_command_pool.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{
void VulkanCommandPool::init()
{

    VkCommandPoolCreateInfo pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = VulkanContext::get_instance().get_vk_device().m_indices.graphics_family.value()};
    if (vkCreateCommandPool(VulkanContext::get_instance().get_vk_device().m_device, &pool_create_info, nullptr,
                            &m_command_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanCommandPool::cleanup()
{
    if (m_command_pool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(VulkanContext::get_instance().get_vk_device().m_device, m_command_pool, nullptr);
        m_command_pool = VK_NULL_HANDLE;
    }
}

} // namespace huan_renderer
