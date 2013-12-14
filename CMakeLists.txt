cmake_minimum_required(VERSION 2.8)

project(craft)

add_executable(
    craft main.c client.c cube.c db.c map.c matrix.c noise.c util.c world.c
    sqlite/sqlite3.c lodepng/lodepng.c tinycthread/tinycthread.c)

add_definitions(-std=c99 -O3 -DPREFIX=\"${CMAKE_INSTALL_PREFIX}\")

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(sqlite)
include_directories(lodepng)
include_directories(tinycthread)
include_directories("/usr/local/include")

install_programs(/bin FILES craft)
install_files(/share/craft/shaders FILES shaders/block_fragment.glsl shaders/block_vertex.glsl
        shaders/line_fragment.glsl shaders/line_vertex.glsl)
install_files(/share/craft FILES texture.png)

if (APPLE)
    target_link_libraries(craft glfw ${GLFW_LIBRARIES})
else()
    find_package(GLEW REQUIRED)
    if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
        target_link_libraries(craft glfw ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})
    else ()
        target_link_libraries(craft glfw dl ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})
    endif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
endif()
