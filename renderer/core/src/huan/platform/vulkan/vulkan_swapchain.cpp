#include "huan/platform/vulkan/vulkan_swapchain.hpp"
#include "huan/core/application.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace huan_renderer
{
void VulkanSwapChain::init()
{
    VulkanContext& context = VulkanContext::get_instance();
    SwapChainSupportDetails swap_chain_support = query_swap_chain_support(context.get_vk_device().m_physical_device);
    m_format = choose_swap_surface_format(swap_chain_support.formats);
    m_present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
    m_extent = choose_swap_extent(swap_chain_support.capabilities);

    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0 &&
        image_count > swap_chain_support.capabilities.maxImageCount)
    {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = context.get_vk_surface().m_surface;

    create_info.minImageCount = image_count;
    create_info.imageFormat = m_format.format;
    create_info.imageColorSpace = m_format.colorSpace;
    create_info.imageExtent = m_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[] = {context.get_vk_device().m_indices.graphics_family.value(),
                                       context.get_vk_device().m_indices.present_family.value()};

    if (context.get_vk_device().m_indices.graphics_family.value() !=
        context.get_vk_device().m_indices.present_family.value())
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = m_present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(context.get_vk_device().m_device, &create_info, nullptr, &m_swap_chain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(context.get_vk_device().m_device, m_swap_chain, &image_count, nullptr);
    std::cout << "Number of swapchain images: " << image_count << std::endl;
    m_images.resize(image_count);
    vkGetSwapchainImagesKHR(context.get_vk_device().m_device, m_swap_chain, &image_count, m_images.data());
}

void VulkanSwapChain::setup_image_views()
{
    m_image_views.resize(m_images.size());
    for (size_t i = 0; i < m_images.size(); i++)
    {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = m_format.format;
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(VulkanContext::get_instance().get_vk_device().m_device, &view_info, nullptr,
                              &m_image_views[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}
void VulkanSwapChain::cleanup()
{
    VulkanContext& context = VulkanContext::get_instance();
    vkDestroySwapchainKHR(context.get_vk_device().m_device, m_swap_chain, nullptr);
}

void VulkanSwapChain::cleanup_image_views()
{
    VulkanContext& context = VulkanContext::get_instance();
    for (auto image_view : m_image_views)
    {
        vkDestroyImageView(context.get_vk_device().m_device, image_view, nullptr);
    }
    m_image_views.clear();
}
SwapChainSupportDetails VulkanSwapChain::query_swap_chain_support(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VulkanContext::get_instance().get_vk_surface().m_surface,
                                              &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanContext::get_instance().get_vk_surface().m_surface,
                                         &format_count, nullptr);

    if (format_count != 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanContext::get_instance().get_vk_surface().m_surface,
                                             &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanContext::get_instance().get_vk_surface().m_surface,
                                              &present_mode_count, nullptr);

    if (present_mode_count != 0)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanContext::get_instance().get_vk_surface().m_surface,
                                                  &present_mode_count, details.present_modes.data());
    }

    return details;
}
VkSurfaceFormatKHR VulkanSwapChain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    for (const auto& available_format : available_formats)
    {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_format;
        }
    }

    return available_formats[0];
}
VkPresentModeKHR VulkanSwapChain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
    for (const auto& available_present_mode : available_present_modes)
    {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D VulkanSwapChain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(Application::get_instance().get_window_handle(), &width, &height);

        VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actual_extent.width =
            std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height =
            std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}
} // namespace huan_renderer
