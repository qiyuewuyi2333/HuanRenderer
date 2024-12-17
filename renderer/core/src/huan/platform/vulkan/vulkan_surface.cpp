#include "huan/platform/vulkan/vulkan_surface.hpp"
#include "GLFW/glfw3.h"
#include "huan/core/application.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{
void VulkanSurface::init()
{
    if (glfwCreateWindowSurface(VulkanContext::get_instance().get_vk_instance(),
                                Application::get_instance().get_window_handle(), nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan surface!");
    }
}
void VulkanSurface::cleanup()
{
    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(VulkanContext::get_instance().get_vk_instance(), m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}

} // namespace huan_renderer
