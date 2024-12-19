#include <iostream>
#include <ostream>
#include <vulkan/vulkan_core.h>
#include "huan/core/application.hpp"
#include "huan/core/config.hpp"
#include "huan/core/create_info.hpp"

namespace huan_renderer
{

Application* Application::instance = nullptr;

Application::Application(ApplicationCreateInfo& app_create_info) : m_app_info(app_create_info)
{
}
void Application::init(ApplicationCreateInfo& app_create_info)
{
    // Initialization code here
    init_window(app_create_info.window_create_info);

    m_renderer = create_scope<Renderer>();
    m_renderer->init();
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
void Application::run()
{
    // Main application loop code here
    while (!glfwWindowShouldClose(m_window_handle))
    {
        draw_frame();
        glfwPollEvents();
    }
}
void Application::shutdown()
{
    m_renderer->shutdown();

    glfwDestroyWindow(m_window_handle);
    glfwTerminate();
}
void Application::draw_frame()
{
    /*
     * 1. Wait for the previous frame to finish
     * 2. Acquire an image from the **swap chain**
     * 3. Record a command buffer which draws the scene onto that image
     * 4. Submit the recorded command buffer
     * 5. Present the swap chain image
     *
     */
    /*
     * Semaphore : When GPU wants to know something finished
     * Fence     : When CPU
     */
    m_renderer->draw();
}

Application& Application::get_instance()
{
    return *instance;
}
} // namespace huan_renderer
