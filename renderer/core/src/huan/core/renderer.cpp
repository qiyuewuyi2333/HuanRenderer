#include "huan/core/renderer.hpp"
#include "huan/core/config.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"

namespace huan_renderer
{
void Renderer::init()
{
    // Renderer just borrow the context from Vulkan
    m_context.reset(&VulkanContext::get_instance());
    m_context->init();
}
void Renderer::shutdown()
{
    m_context->cleanup();
}

} // namespace huan_renderer
