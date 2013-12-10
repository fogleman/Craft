cmake_minimum_required(VERSION 2.8)

project(craft)

add_executable(
    craft main.c client.c db.c map.c noise.c util.c
    sqlite/sqlite3.c lodepng/lodepng.c tinycthread/tinycthread.c)

add_definitions(-std=c99 -O3)

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(sqlite)
include_directories(lodepng)
include_directories(tinycthread)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules/")
set(GLEW_LOCATION "${CMAKE_SOURCE_DIR}/glew-1.10.0")

if (APPLE)
    target_link_libraries(craft glfw ${GLFW_LIBRARIES})
else()
    find_package(GLEW REQUIRED)
    target_link_libraries(craft glfw dl ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})
endif()
