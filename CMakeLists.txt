cmake_minimum_required(VERSION 3.5)

project(craft C)

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS src/*.c)

add_executable(craft
    ${SOURCE_FILES}
    deps/glew/src/glew.c
    deps/lodepng/lodepng.c
    deps/noise/noise.c
    deps/sqlite/sqlite3.c
    deps/tinycthread/tinycthread.c
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-O3)

add_subdirectory(deps/glfw)

include_directories(
    deps/glew/include
    deps/glfw/include
    deps/lodepng
    deps/noise
    deps/sqlite
    deps/tinycthread
)

if(MINGW)
    list(APPEND CMAKE_LIBRARY_PATH
        "C:/Program Files/CURL/lib"
        "C:/Program Files (x86)/CURL/lib"
    )
    list(APPEND CMAKE_INCLUDE_PATH
        "C:/Program Files/CURL/include"
        "C:/Program Files (x86)/CURL/include"
    )
endif()

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
set(OpenGL_GL_PREFERENCE "LEGACY")

if(APPLE)
    target_link_libraries(craft
        glfw
        OpenGL::GL
        ${GLFW_LIBRARIES}
        ${CURL_LIBRARIES}
    )
elseif(UNIX)
    target_link_libraries(craft
        dl
        glfw
        OpenGL::GL
        ${GLFW_LIBRARIES}
        ${CURL_LIBRARIES}
    )
elseif(MINGW)
    target_link_libraries(craft
        ws2_32
        glfw
        ${GLFW_LIBRARIES}
        ${CURL_LIBRARIES}
    )
endif()
