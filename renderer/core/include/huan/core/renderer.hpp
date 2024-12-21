#include "huan/platform/vulkan/vulkan_api.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>
namespace huan_renderer
{

class Renderer
{

  public:
    Renderer() = default;
    inline VulkanContext& get_context()
    {
        return *m_context;
    }
    virtual void init();
    virtual void shutdown();
    virtual void draw();
    virtual void wait_idle();

  private:
    VulkanContext* m_context;
    Scope<VulkanAPI> m_render_api;
};

} // namespace huan_renderer
