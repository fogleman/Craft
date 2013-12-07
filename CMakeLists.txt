cmake_minimum_required(VERSION 2.8)

project(craft)

add_executable(craft main.c db.c map.c noise.c util.c sqlite/sqlite3.c lodepng/lodepng.c)

add_definitions(-std=c99 -O3)

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(sqlite)
include_directories(lodepng)

target_link_libraries(craft glfw ${GLFW_LIBRARIES})
