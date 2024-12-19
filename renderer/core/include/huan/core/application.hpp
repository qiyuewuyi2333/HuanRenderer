#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "huan/core/config.hpp"
#include "huan/core/create_info.hpp"
#include <huan/core/renderer.hpp>

namespace huan_renderer
{

class HUAN_API Application
{
  public:
    Application(ApplicationCreateInfo& create_info);
    HUAN_NO_COPY(Application)
    HUAN_NO_MOVE(Application)
    ~Application() = default;

    static Application& get_instance();
    inline const ApplicationCreateInfo& get_app_info()
    {
        return m_app_info;
    }
    inline GLFWwindow* get_window_handle()
    {
        return m_window_handle;
    }

    virtual void run();

    virtual void shutdown();

    virtual void draw_frame();

  public:
    static Application* instance;
    void init(ApplicationCreateInfo& create_info);
    void init_window(WindowCreateInfo& window_create_info);

  protected:
    ApplicationCreateInfo m_app_info;
    Scope<Renderer> m_renderer;
    GLFWwindow* m_window_handle;
};
void create_instance(ApplicationCreateInfo& app_create_info);

} // namespace huan_renderer
