#pragma once
#include "huan/core/config.hpp"
#include "huan/core/create_info.hpp"
#include "huan/core/renderer_context.hpp"
#include "huan/platform/vulkan/vulkan_device.hpp"
#include <optional>
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
    virtual void shutdown();
    VkInstance& get_vk_instance();

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

    Ref<ApplicationCreateInfo> m_app_info;
};
} // namespace huan_renderer
