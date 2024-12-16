#pragma once
#include <string>
#include <vector>

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
#ifdef HUAN_DEBUG_MODE
    const bool enable_validation_layers = true;
#else
    const bool enable_validation_layers = false;
#endif
};
const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
} // namespace huan_renderer
