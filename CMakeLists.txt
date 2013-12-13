cmake_minimum_required(VERSION 2.8)

project(craft)

add_executable(
    craft main.c client.c cube.c db.c map.c matrix.c noise.c util.c world.c
    sqlite/sqlite3.c lodepng/lodepng.c tinycthread/tinycthread.c glew/src/glew.c)

if (!MSVC)
    add_definitions(-std=c99)
endif()

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(glew/include)
include_directories(sqlite)
include_directories(lodepng)
include_directories(tinycthread)

if (APPLE)
    target_link_libraries(craft glfw ${GLFW_LIBRARIES})
elif(LINUX)
    target_link_libraries(craft glfw dl ${GLFW_LIBRARIES})
else()
    target_link_libraries(craft glfw ws2_32.lib ${GLFW_LIBRARIES})
endif()
