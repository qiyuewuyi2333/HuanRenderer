#pragma once
#include "GLFW/glfw3.h"
#include "huan/core/config.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    inline bool is_complete()
    {
        return graphics_family.has_value() && present_family.has_value();
    }
};

/*
 * How to enable an extension?
 * Just make a small change in logical device.
 *
 *
 *
 *
 */
class VulkanDevice
{
  public:
    VulkanDevice() = default;
    HUAN_NO_COPY(VulkanDevice)
    HUAN_NO_MOVE(VulkanDevice)
    ~VulkanDevice() = default;

    void init();
    void cleanup();

  private:
    bool is_device_suitable(VkPhysicalDevice device);
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
    bool check_device_extension_support(VkPhysicalDevice device);

  public:
    VkDevice m_device;
    VkPhysicalDevice m_physical_device;
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;
    QueueFamilyIndices m_indices;

    // Other members and functions...
};

} // namespace huan_renderer
