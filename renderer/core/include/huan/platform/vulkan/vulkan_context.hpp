#pragma once
#include "huan/core/config.hpp"
#include "huan/core/create_info.hpp"
#include "huan/platform/vulkan/vulkan_device.hpp"
#include "huan/platform/vulkan/vulkan_pipeline.hpp"
#include "huan/platform/vulkan/vulkan_renderpass.hpp"
#include "huan/platform/vulkan/vulkan_surface.hpp"
#include "huan/platform/vulkan/vulkan_surface.hpp"
#include "huan/platform/vulkan/vulkan_swapchain.hpp"
#include <vulkan/vulkan_core.h>

struct GLFWwindow;

namespace huan_renderer
{

class VulkanContext
{
  public:
    VulkanContext();
    virtual ~VulkanContext() = default;

    // Singleton
    static VulkanContext& get_instance();

    virtual void init();
    virtual void cleanup();
    VkInstance& get_vk_instance();
    VulkanDevice& get_vk_device();
    VulkanSurface& get_vk_surface();
    VulkanSwapChain& get_vk_swapchain();
    VulkanRenderPass& get_vk_render_pass();

  private:
    void init_vulkan();
    void init_vk_instance();
    void init_vk_debug_messenger();
    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
    std::vector<const char*> get_required_extensions();

  private:
    static Scope<VulkanContext> m_instance;

    VkInstance m_vk_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VulkanDevice m_device;
    VulkanSurface m_surface;
    VulkanSwapChain m_swapchain;
    VulkanRenderPass m_render_pass;

    VulkanPipeline m_pipeline;

    Ref<ApplicationCreateInfo> m_app_info;
};
} // namespace huan_renderer
