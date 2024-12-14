#pragma once

#include "huan/core/config.hpp"
#include "huan/window/window.hpp"
#include <cassert>
#include <string>
namespace huan_renderer
{

class Application
{
  public:
    Application(const ApplicationCreateInfo& create_info);
    HUAN_NO_COPY(Application)
    HUAN_NO_MOVE(Application)
    ~Application();

    inline static void create_instance(const ApplicationCreateInfo& create_info)
    {
        if (!instance)
        {
            instance = new Application(create_info);
        }
    }
    inline static Application* get_instance()
    {
        return instance;
    }

    virtual void run();

  private:
    virtual void init();
    virtual void shutdown();
    virtual void processInput();
    virtual void update();
    inline static Application* instance = nullptr;

  private:
    ApplicationCreateInfo createInfo;
    Scope<Window> m_window;
};
} // namespace huan_renderer
