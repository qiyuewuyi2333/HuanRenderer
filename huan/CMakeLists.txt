project(Renderer)
message("Current Project name: " ${PROJECT_NAME})

option(HUAN_ENABLE_LOG "Enable log" ON)
option(HUAN_ENABLE_ASSERT "Enable assert" ON)

add_library(${PROJECT_NAME} SHARED
        src/HelloTriangleApplication.cpp
        include/huan/HelloTriangleApplication.hpp
        include/huan/settings.hpp
        include/huan/common.hpp
        src/settings.cpp
        include/huan/log/Log.hpp
        src/log/Log.cpp
        include/huan/backend/swapchain.hpp
        src/backend/swapchain.cpp
        src/utils/file_load.cpp
        include/huan/utils/file_load.hpp
        src/backend/shader.cpp
        include/huan/backend/shader.hpp
        include/huan/backend/vulkan_buffer.hpp
        src/backend/vulkan_buffer.cpp
        src/utils/stb_image.cpp
        include/huan/backend/vulkan_image.hpp
        src/backend/vulkan_image.cpp
        include/huan/backend/vulkan_command.hpp
        src/backend/vulkan_command.cpp
        include/huan/common_templates/deferred_system.hpp
        include/huan/backend/vulkan_resources.hpp
        src/backend/vulkan_resources.cpp)

if(${HUAN_ENABLE_LOG})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HUAN_ENABLE_LOG)
endif ()
if(${HUAN_ENABLE_ASSERT})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HUAN_ENABLE_ASSERT)
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE HUAN_BUILD_SHARED)
message( STATUS "${PROJECT_NAME} should be built to dll.")
target_compile_definitions(${PROJECT_NAME} PRIVATE HUAN_INNER_VISIBLE)
message( STATUS "Some classes in ${PROJECT_NAME} should be visible inside.") 

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
