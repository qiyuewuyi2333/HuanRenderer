#pragma once
#include "huan/core/config.hpp"
#include <optional>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphics_family;
    // std::optional<uint32_t> present_family;
    inline bool is_complete()
    {
        return graphics_family.has_value();
    }
};
class VulkanDevice
{
  public:
    VulkanDevice();
    HUAN_NO_COPY(VulkanDevice)
    HUAN_NO_MOVE(VulkanDevice)
    ~VulkanDevice() = default;

    void init();
    void shutdown();

  public:
    VkDevice m_device;
    VkPhysicalDevice m_physical_device;
    VkQueue m_graphics_queue;
    QueueFamilyIndices m_indices;

    // Other members and functions...
};
bool is_device_suitable(VkPhysicalDevice device);
QueueFamilyIndices find_queue_families(VkPhysicalDevice device);

} // namespace huan_renderer
