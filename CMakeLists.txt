cmake_minimum_required(VERSION 3.0)
project(HuanRenderer)

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using VCPKG for C++ package management!")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(sandbox)
add_subdirectory(huan)
add_subdirectory(third_party/VulkanMemoryAllocator)

