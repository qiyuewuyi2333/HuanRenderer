project(Renderer)
message("Current Project name: " ${PROJECT_NAME})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    option(HUAN_DEBUG "Debug mode" ON)
else ()
    option(HUAN_DEBUG "Debug mode" OFF)
endif ()

option(HUAN_ENABLE_LOG "Enable log" ON)
option(HUAN_ENABLE_ASSERT "Enable assert" ON)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SRC_FILES}
        include/huan/scene_framework/components/mesh.hpp
        src/framework/components/mesh.cpp
        src/framework/components/AABB3D.cpp
        include/huan/scene_framework/components/AABB3D.hpp)

if (${HUAN_DEBUG})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HUAN_DEBUG)
    message(STATUS "Build in Debug mode")
endif ()

if (${HUAN_ENABLE_LOG})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HUAN_ENABLE_LOG)
endif ()
if (${HUAN_ENABLE_ASSERT})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HUAN_ENABLE_ASSERT)
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE HUAN_BUILD_SHARED)
message(STATUS "${PROJECT_NAME} should be built to dll.")
target_compile_definitions(${PROJECT_NAME} PRIVATE HUAN_INNER_VISIBLE)
message(STATUS "Some classes in ${PROJECT_NAME} should be visible inside.")

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)

# VMA
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/third_party/VulkanMemoryAllocator/include)
target_link_libraries(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)

# GLSLang
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/third_party/glslang)
target_link_libraries(${PROJECT_NAME} PRIVATE
        glslang
        glslang-default-resource-limits
)

# spirv_cross
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_util CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_c CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-core)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-glsl)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-hlsl)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-reflect)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-util)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-msl)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-cross-c)


find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)