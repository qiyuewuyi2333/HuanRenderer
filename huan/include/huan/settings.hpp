//
// Created by 86156 on 4/4/2025.
//

#ifndef CONFIG_HPP
#define CONFIG_HPP
#include "huan/common.hpp"

namespace huan
{
    struct AppSettings
    {
        const char* title = "Default title";
        int width = 800;
        int height = 600;
        bool isVulkanValidationEnabled = true;
        int maxFramesInFlight = 2;
    };

   HUAN_API extern  AppSettings globalAppSettings;
}

#endif //CONFIG_HPP
