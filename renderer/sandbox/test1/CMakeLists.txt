cmake_minimum_required (VERSION 3.8)


project ("test1")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)


add_executable(test1 main.cpp)

target_compile_definitions(test1 PRIVATE HUAN_DLL_IMPORT)
target_include_directories(test1 PRIVATE ${CMAKE_SOURCE_DIR}/core/include)

message("include: " ${CMAKE_SOURCE_DIR}/core/include)

# Should also has target_link_libraries
target_link_libraries(test1 PRIVATE RenderCore)
