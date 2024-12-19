#include "huan/platform/vulkan/vulkan_context.hpp"
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

  private:
    void create_sync_objects();

  private:
    VulkanContext* m_context;
    VkSemaphore m_image_available_semaphore;
    VkSemaphore m_render_finished_semaphore;
    VkFence m_in_flight_fence;
};

} // namespace huan_renderer
