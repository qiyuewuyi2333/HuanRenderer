#include "huan/core/application.hpp"
#include "huan/core/create_info.hpp"
#include <iostream>

class SandboxTest : public huan_renderer::Application
{
  public:
    using huan_renderer::Application::Application;
    void run() override
    {
        std::cout << "SandboxTest is running" << std::endl;
        while (!glfwWindowShouldClose(m_window_handle))
        {
            glfwPollEvents();
        }
    }
};
void huan_renderer::create_instance(ApplicationCreateInfo& app_create_info)
{
    huan_renderer::Application::instance = new SandboxTest(app_create_info);
}

int main(int argc, char* argv[])
{

    huan_renderer::ApplicationCreateInfo app_create_info;
    huan_renderer::create_instance(app_create_info);

    huan_renderer::Application::get_instance()->init(app_create_info);
    huan_renderer::Application::get_instance()->run();
    huan_renderer::Application::get_instance()->shutdown();
    return 0;
}
