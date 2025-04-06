#include <iostream>

#include "huan/HelloTriangleApplication.hpp"
#include "huan/settings.hpp"

int main()
{
    huan::globalAppSettings = {
        .title = "Hello Triangle",
        .width = 800,
        .height = 600,
        
    };

    huan::HelloTriangleApplication* app = huan::HelloTriangleApplication::getInstance();

    try
    {
        app->init();
        app->run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    app->cleanup();
    
    return EXIT_SUCCESS;
}
