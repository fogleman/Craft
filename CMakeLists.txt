cmake_minimum_required(VERSION 2.8)

project(craft)

add_executable(
    craft main.c client.c cube.c db.c map.c matrix.c noise.c util.c world.c
    sqlite/sqlite3.c lodepng/lodepng.c tinycthread/tinycthread.c)

if (MSVC)
    add_definitions(-W3 -MD -DWIN32 -D_CRT_SECURE_NO_WARNINGS)
else()
    add_definitions(-std=c99 -O3)
endif()

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(sqlite)
include_directories(lodepng)
include_directories(tinycthread)

if (APPLE)
    target_link_libraries(craft glfw ${GLFW_LIBRARIES})
elseif (WIN32)
    include_directories(glew-1.10.0/include)
    target_link_libraries(craft glfw glew-1.10.0/lib/Release/Win32/glew32 ws2_32 ${GLFW_LIBRARIES})
else()
    find_package(GLEW REQUIRED)
    target_link_libraries(craft glfw dl ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})
endif()
