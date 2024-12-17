#include "huan/platform/vulkan/vulkan_context.hpp"
#include "huan/core/application.hpp"
#include "huan/core/config.hpp"
#include "huan/core/create_info.hpp"
#include "huan/platform/vulkan/vulkan_device.hpp"
#include "huan/platform/vulkan/vulkan_surface.hpp"
#include <cstdint>
#include <set>
#include <string>
#include <urlmon.h>
#include <vector>
#include <iostream>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{
bool check_validation_layer_support();
void print_available_extensions();
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator);

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_call_back(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                      VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                      const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
                                                      void* p_user_data)
{

    std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;

    return VK_FALSE;
}
Scope<VulkanContext> VulkanContext::m_instance = nullptr;

VulkanContext::VulkanContext()
    : m_app_info(create_ref<ApplicationCreateInfo>(Application::get_instance().get_app_info()))
{
}

VulkanContext& VulkanContext::get_instance()
{
    if (m_instance.get() == nullptr)
        m_instance.reset(new VulkanContext());

    return *m_instance;
}
void VulkanContext::init()
{
    init_vulkan();
}

void VulkanContext::init_vulkan()
{
    // Initialization code here
    init_vk_instance();
    init_vk_debug_messenger();
    m_surface.init();
    m_device.init();
    m_swapchain.init();
    m_swapchain.setup_image_views();
}

void VulkanContext::init_vk_instance()
{
    VkApplicationInfo vk_app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                  .pNext = nullptr,
                                  .pApplicationName = m_app_info->app_name.data(),
                                  .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                  .pEngineName = "No Engine",
                                  .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                  .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                  .apiVersion = VK_API_VERSION_1_0};
    VkInstanceCreateInfo vk_instance_create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &vk_app_info,
    };

    // Validation layer check if needed
    if (m_app_info->enable_validation_layers && !check_validation_layer_support())
    {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    print_available_extensions();
    if (m_app_info->enable_validation_layers)
    {
        std::cout << "Validation layers enabled" << std::endl;
        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        populate_debug_messenger_create_info(debug_create_info);
        vk_instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
        vk_instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        vk_instance_create_info.ppEnabledLayerNames = validation_layers.data();
    }
    else
    {
        vk_instance_create_info.enabledLayerCount = 0;
        vk_instance_create_info.pNext = nullptr;
    }
    // enable needed extensions
    std::vector<const char*> extensions = get_required_extensions();
    vk_instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    vk_instance_create_info.ppEnabledExtensionNames = extensions.data();
    // print required extensions
    std::cout << "Required Vulkan extensions:\n";
    for (const auto& extension : extensions)
    {
        std::cout << "\t" << extension << std::endl;
    }

    VkResult vk_result = vkCreateInstance(&vk_instance_create_info, nullptr, &m_vk_instance);
    if (vk_result != VK_SUCCESS)
    {
        std::cerr << "Failed to create Vulkan instance!" << std::endl;
        return;
    }
}
void VulkanContext::init_vk_debug_messenger()
{
    if (!m_app_info->enable_validation_layers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    populate_debug_messenger_create_info(debug_create_info);

    if (CreateDebugUtilsMessengerEXT(m_vk_instance, &debug_create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void VulkanContext::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
    create_info = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                   .pNext = nullptr,
                   .flags = 0,
                   .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                   .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                   .pfnUserCallback = debug_call_back,
                   .pUserData = nullptr};
}

VkInstance& VulkanContext::get_vk_instance()
{
    return m_vk_instance;
}
VulkanDevice& VulkanContext::get_vk_device()
{
    return m_device;
}
VulkanSurface& VulkanContext::get_vk_surface()
{
    return m_surface;
}
void VulkanContext::shutdown()
{
    m_swapchain.cleanup_image_views();
    m_swapchain.cleanup();
    m_device.cleanup();
    m_surface.cleanup();
    if (m_app_info->enable_validation_layers)
    {
        DestroyDebugUtilsMessengerEXT(m_vk_instance, m_debug_messenger, nullptr);
    }
    vkDestroyInstance(m_vk_instance, nullptr);
}

bool check_validation_layer_support()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : validation_layers)
    {
        bool layer_found = false;
        for (const auto& layer_properties : available_layers)
        {
            if (strcmp(layer_name, layer_properties.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }
        if (!layer_found)
        {
            return false;
        }
    }
    return true;
}
std::vector<const char*> VulkanContext::get_required_extensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (m_app_info->enable_validation_layers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}
void print_available_extensions()
{
    uint32_t extensions_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extensions_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, extensions.data());
    std::cout << "Available Vulkan extensions:\n";
    for (const auto& extension : extensions)
    {
        std::cout << "\tExtension: " << extension.extensionName << std::endl;
    }
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}
} // namespace huan_renderer
