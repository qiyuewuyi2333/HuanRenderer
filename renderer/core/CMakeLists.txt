cmake_minimum_required (VERSION 3.8)


project ("RenderCore")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)





# This project is for to generate a dll

file(GLOB_RECURSE RENDERCORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/huan/*.cpp")

add_library(${PROJECT_NAME} SHARED  ${RENDERCORE_SOURCES})

message("PROJECT_NAME: " ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PRIVATE HUAN_DLL_EXPORT)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)



