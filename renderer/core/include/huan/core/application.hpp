#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "huan/core/config.hpp"
#include "huan/core/create_info.hpp"

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

    virtual void run();

    virtual void shutdown();

    std::vector<const char*> get_required_extensions();

  public:
    static Application* instance;
    void init(ApplicationCreateInfo& create_info);
    void init_window(WindowCreateInfo& window_create_info);
    void init_vulkan();
    void init_vk_instance();
    void init_vk_debug_messenger();
    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);

  protected:
    ApplicationCreateInfo m_app_info;
    VkInstance m_vk_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger;
    GLFWwindow* m_window_handle;
};
void create_instance(ApplicationCreateInfo& app_create_info);

} // namespace huan_renderer
