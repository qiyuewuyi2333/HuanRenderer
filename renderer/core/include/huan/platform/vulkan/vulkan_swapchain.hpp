#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
namespace huan_renderer
{
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};
class VulkanSwapChain
{
  public:
    VulkanSwapChain() = default;
    ~VulkanSwapChain() = default;

    void init();
    void cleanup();

    void setup_image_views();
    void cleanup_image_views();

  private:
    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

  public:
    VkSwapchainKHR m_swap_chain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_image_views;

    VkSurfaceFormatKHR m_format;
    VkPresentModeKHR m_present_mode;
    VkExtent2D m_extent;
};

} // namespace huan_renderer
