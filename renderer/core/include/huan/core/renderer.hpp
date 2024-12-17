#include "huan/core/config.hpp"
#include "huan/core/renderer_context.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
namespace huan_renderer
{

class Renderer
{

  public:
    Renderer() = default;
    inline VulkanContext& get_context()
    {
        return *m_context.get();
    }
    virtual void init();
    virtual void shutdown();

  private:
    Ref<VulkanContext> m_context;
};

} // namespace huan_renderer
