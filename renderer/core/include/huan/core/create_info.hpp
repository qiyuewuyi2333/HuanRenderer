#pragma once
#include <string>

namespace huan_renderer
{
struct WindowCreateInfo
{
    std::string title = "[default name]";
    int width = 1080;
    int height = 720;
    bool fullscreen = false;
};
struct ApplicationCreateInfo
{
    std::string app_name = "[default name]";
    WindowCreateInfo window_create_info;
};

} // namespace huan_renderer
