#include "huan/core/renderer.hpp"
#include "huan/core/config.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{
void Renderer::init()
{
    // Renderer just borrow the context from Vulkan
    m_context = &VulkanContext::get_instance();
    m_context->init();
    m_render_api = create_scope<VulkanAPI>();
    m_render_api->init();
}

void Renderer::shutdown()
{
    m_context->cleanup();
}

void Renderer::draw()
{
    m_render_api->draw();
}

void Renderer::wait_idle()
{
    vkDeviceWaitIdle(m_context->get_vk_device().m_device);
}
} // namespace huan_renderer
