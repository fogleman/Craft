cmake_minimum_required(VERSION 2.8)

project(konstructs)

FILE(GLOB SOURCE_FILES src/*.c)

add_executable(
    konstructs
    ${SOURCE_FILES}
    deps/glew/src/glew.c
    deps/lodepng/lodepng.c
    deps/noise/noise.c
    deps/tinycthread/tinycthread.c)

add_definitions(-std=c99 -O3)

add_subdirectory(deps/glfw)
include_directories(deps/glew/include)
include_directories(deps/glfw/include)
include_directories(deps/lodepng)
include_directories(deps/noise)
include_directories(deps/tinycthread)

set(CMAKE_BUILD_TYPE Debug)

find_package(ZLIB REQUIRED)

include_directories(${ZLIB_INCLUDE_DIRS})

if(APPLE)
    target_link_libraries(konstructs glfw
        ${GLFW_LIBRARIES} ${ZLIB_LIBRARIES})
endif()

if(UNIX)
    target_link_libraries(konstructs dl glfw
        ${GLFW_LIBRARIES} ${ZLIB_LIBRARIES})
endif()

if(MINGW)
    target_link_libraries(konstructs ws2_32.lib glfw
        ${GLFW_LIBRARIES} ${ZLIB_LIBRARIES})
endif()
