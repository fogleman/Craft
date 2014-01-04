cmake_minimum_required(VERSION 2.8)

project(craft)

add_executable(
    craft main.c client.c cube.c db.c map.c matrix.c ring.c util.c clouds.c world.c
    mob/mob.c mob/butterfly.c 
    glew/src/glew.c lodepng/lodepng.c noise/noise.c sqlite/sqlite3.c
    tinycthread/tinycthread.c)


add_definitions(-std=c99 -O3)

add_subdirectory(glfw)
include_directories(glew/include)
include_directories(glfw/include)
include_directories(lodepng)
include_directories(noise)
include_directories(sqlite)
include_directories(tinycthread)

if(APPLE)
    target_link_libraries(craft glfw ${GLFW_LIBRARIES})
elseif(UNIX)
    target_link_libraries(craft glfw dl ${GLFW_LIBRARIES})
else()
    target_link_libraries(craft glfw ws2_32.lib ${GLFW_LIBRARIES})
endif()
