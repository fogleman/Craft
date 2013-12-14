cmake_minimum_required(VERSION 2.8)

project(craft)

add_executable(
    craft main.c client.c cube.c db.c map.c matrix.c noise.c util.c world.c
    sqlite/sqlite3.c lodepng/lodepng.c tinycthread/tinycthread.c)

add_definitions(-std=c99 -O3)

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(sqlite)
include_directories(lodepng)
include_directories(tinycthread)
if (APPLE)
    target_link_libraries(craft glfw ${GLFW_LIBRARIES})
else(WIN32)
    find_package(GLEW REQUIRED)
    target_link_libraries(craft glfw dl ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} ws2_32)
else()
    find_package(GLEW REQUIRED)
    target_link_libraries(craft glfw dl ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})
endif()
