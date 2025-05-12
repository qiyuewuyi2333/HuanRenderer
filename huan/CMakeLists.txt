project(Renderer)
message("Current Project name: " ${PROJECT_NAME})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    option(HUAN_DEBUG "Debug mode" ON)
else ()
    option(HUAN_DEBUG "Debug mode" OFF)
endif ()

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
        include/huan/backend/resource/vulkan_resources.hpp
        src/backend/resource/vulkan_resources.cpp
        src/backend/vmaUsage.cpp
        src/utils/tiny_obj_loader_usage.cpp
        src/framework/component.cpp
        include/huan/framework/component.hpp
        src/framework/node.cpp
        include/huan/framework/node.hpp
        src/framework/scene.cpp
        include/huan/framework/scene.hpp
        src/framework/components/sub_mesh.cpp
        include/huan/framework/components/sub_mesh.hpp
        src/framework/components/material.cpp
        include/huan/framework/components/material.hpp
        src/framework/script.cpp
        include/huan/framework/script.hpp
        src/utils/file_system.cpp
        include/huan/utils/file_system.hpp
        src/backend/shader/glsl_compiler.cpp
        include/huan/backend/shader/glsl_compiler.hpp
        src/backend/shader/spirv_reflection.cpp
        include/huan/backend/shader/spirv_reflection.hpp
        src/framework/components/texture.cpp
        include/huan/framework/components/texture.hpp
        include/huan/backend/resource/vulkan_resource.hpp
        include/huan/backend/resource/sampler.hpp
        src/backend/resource/sampler.cpp
        include/huan/backend/resource/allocated.hpp
)

if(${HUAN_DEBUG})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HUAN_DEBUG)
    message(STATUS "Build in Debug mode")
endif ()

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

# VMA
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/third_party/VulkanMemoryAllocator/include)
target_link_libraries(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)

# GLSLang
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/third_party/glslang)
target_link_libraries(${PROJECT_NAME} PRIVATE glslang)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
