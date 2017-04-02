cmake_minimum_required(VERSION 2.8)

project(craft)

FILE(GLOB SOURCE_FILES src/*.c)

if (EMSCRIPTEN)
    add_executable(
        craft
        ${SOURCE_FILES}
        #deps/glew/src/glew.c
        deps/lodepng/lodepng.c
        deps/noise/noise.c
        deps/sqlite/sqlite3.c
        deps/tinycthread/tinycthread.c)
else ()
    add_executable(
        craft
        ${SOURCE_FILES}
        deps/glew/src/glew.c
        deps/lodepng/lodepng.c
        deps/noise/noise.c
        deps/sqlite/sqlite3.c
        deps/tinycthread/tinycthread.c)
endif ()

add_definitions(-std=c99 -O3)

if (EMSCRIPTEN)
    # Emscripten default is GLFW 2.x but we use GLFW 3.x, also include data files
    set_target_properties(craft PROPERTIES LINK_FLAGS "-s USE_GLFW=3 --embed-file ../shaders/")
endif ()

if (NOT EMSCRIPTEN)
    # Emscripten includes its own GLFW and GLEW ports
    add_subdirectory(deps/glfw)
    include_directories(deps/glew/include)
    include_directories(deps/glfw/include)
endif ()

include_directories(deps/lodepng)
include_directories(deps/noise)
include_directories(deps/sqlite)
include_directories(deps/tinycthread)

if(MINGW)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
        "C:/Program Files/CURL/lib" "C:/Program Files (x86)/CURL/lib")
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
        "C:/Program Files/CURL/include" "C:/Program Files (x86)/CURL/include")
endif()

if (NOT EMSCRIPTEN)
    find_package(CURL REQUIRED)
    include_directories(${CURL_INCLUDE_DIR})
else ()
    set(CURL_LIBRARIES "")
endif ()

if(APPLE)
    target_link_libraries(craft glfw
        ${GLFW_LIBRARIES} ${CURL_LIBRARIES})
endif()

if(UNIX)
    target_link_libraries(craft dl glfw
        ${GLFW_LIBRARIES} ${CURL_LIBRARIES})
endif()

if(MINGW)
    target_link_libraries(craft ws2_32.lib glfw
        ${GLFW_LIBRARIES} ${CURL_LIBRARIES})
endif()
