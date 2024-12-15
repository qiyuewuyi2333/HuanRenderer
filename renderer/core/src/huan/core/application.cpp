#include <iostream>
#include <ostream>
#define GLFW_INCLUDDE_VULKAN
#include <GLFW/glfw3.h>
#include "huan/core/application.hpp"
#include "huan/core/create_info.hpp"

namespace huan_renderer
{
Application* Application::instance = nullptr;

Application::Application(ApplicationCreateInfo& app_create_info) : m_create_info(app_create_info)
{
}
void Application::init(ApplicationCreateInfo& app_create_info)
{
    // Initialization code here
    init_window(app_create_info.window_create_info);
    init_vulkan();
}
void Application::init_window(WindowCreateInfo& window_create_info)
{

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window_handle = glfwCreateWindow(window_create_info.width, window_create_info.height,
                                       window_create_info.title.data(), nullptr, nullptr);
    if (!m_window_handle)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return;
    }
}
void Application::init_vulkan()
{
    // Initialization code here
}

void Application::run()
{
    // Main application loop code here
    while (!glfwWindowShouldClose(m_window_handle))
    {
        glfwPollEvents();
    }
}
void Application::shutdown()
{
    // Cleanup code here
    glfwDestroyWindow(m_window_handle);
    glfwTerminate();
}

Application* Application::get_instance()
{
    return instance;
}

} // namespace huan_renderer
