cmake_minimum_required(VERSION 2.8)

project(craft)

add_executable(craft main.c noise.c util.c)

add_definitions(-std=c99 -O3)

add_subdirectory(glfw)
include_directories(glfw/include)

target_link_libraries(craft glfw ${GLFW_LIBRARIES})
