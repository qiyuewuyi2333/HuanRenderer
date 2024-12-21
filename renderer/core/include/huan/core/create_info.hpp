#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

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
    const int max_frames_in_flight = 2;
};
const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
} // namespace huan_renderer
