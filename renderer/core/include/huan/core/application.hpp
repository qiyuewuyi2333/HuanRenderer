#pragma once

#include "GLFW/glfw3.h"
#include "huan/core/config.hpp"
#include "huan/core/create_info.hpp"
#include <type_traits>

namespace huan_renderer
{

class HUAN_API Application
{
  public:
    Application(ApplicationCreateInfo& create_info);
    HUAN_NO_COPY(Application)
    HUAN_NO_MOVE(Application)
    ~Application();

    static Application* get_instance();

    virtual void init(ApplicationCreateInfo& create_info);
    virtual void run();

    virtual void init_window(WindowCreateInfo& window_create_info);
    virtual void init_vulkan();
    virtual void shutdown();

  public:
    static Application* instance;

  protected:
    ApplicationCreateInfo m_create_info;
    GLFWwindow* m_window_handle;
};
void create_instance(ApplicationCreateInfo& app_create_info);

} // namespace huan_renderer
