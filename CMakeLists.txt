cmake_minimum_required(VERSION 3.20)
project(craft C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")

add_executable(craft
    ${SOURCE_FILES}
    deps/glew/src/glew.c
    deps/lodepng/lodepng.c
    deps/noise/noise.c
    deps/sqlite/sqlite3.c
    deps/tinycthread/tinycthread.c
)

target_include_directories(craft
    PRIVATE
        ${CMAKE_SOURCE_DIR}/deps/glew/include
        ${CMAKE_SOURCE_DIR}/deps/glfw/include
        ${CMAKE_SOURCE_DIR}/deps/lodepng
        ${CMAKE_SOURCE_DIR}/deps/noise
        ${CMAKE_SOURCE_DIR}/deps/sqlite
        ${CMAKE_SOURCE_DIR}/deps/tinycthread
)

# Build glfw from source (assumes deps/glfw contains a CMakeLists that defines a target)
add_subdirectory(deps/glfw)

# Help find curl on Windows if needed
if(MINGW)
    list(APPEND CMAKE_PREFIX_PATH
        "C:/Program Files/CURL" "C:/Program Files (x86)/CURL"
    )
endif()

find_package(OpenGL REQUIRED)
find_package(CURL REQUIRED)

# Prefer imported targets when available
if(TARGET CURL::libcurl)
    set(CURL_TARGET CURL::libcurl)
else()
    set(CURL_TARGET "")
endif()

# Link libraries (modern target usage). Link OpenGL so GL symbols and GLX/EGL resolvers are available.
target_link_libraries(craft
    PRIVATE
        glfw
        OpenGL::GL
)

# Link curl via imported target or legacy variables
if(CURL_TARGET)
    target_link_libraries(craft PRIVATE ${CURL_TARGET})
else()
    target_include_directories(craft PRIVATE ${CURL_INCLUDE_DIRS})
    target_link_libraries(craft PRIVATE ${CURL_LIBRARIES})
endif()

# Platform-specific linking
if(UNIX AND NOT APPLE)
    target_compile_definitions(craft PRIVATE _UNIX)
    # dl is sometimes needed for dynamic loader symbols
    target_link_libraries(craft PRIVATE dl)
    # On X11/GLX systems, ensure X11 libraries are available if GLFW didn't pull them in
    find_package(X11 QUIET)
    if(X11_FOUND)
        target_link_libraries(craft PRIVATE ${X11_LIBRARIES})
        target_include_directories(craft PRIVATE ${X11_INCLUDE_DIR})
    endif()
elseif(APPLE)
    target_compile_definitions(craft PRIVATE _APPLE)
    # macOS typically uses its own frameworks (OpenGL::GL covers linking). Add frameworks if needed:
    # find_library(COCOA_FRAMEWORK Cocoa)
    # target_link_libraries(craft PRIVATE ${COCOA_FRAMEWORK})
endif()

if(MINGW)
    target_link_libraries(craft PRIVATE ws2_32)
endif()

# Compiler optimization flags
target_compile_options(craft PRIVATE $<$<C_COMPILER_ID:GNU,Clang>:-O3>)

# If your system provides GLEW as a findable package, prefer it instead of compiling glew.c:
# find_package(GLEW QUIET)
# if(GLEW_FOUND)
#     target_include_directories(craft PRIVATE ${GLEW_INCLUDE_DIRS})
#     target_link_libraries(craft PRIVATE ${GLEW_LIBRARIES})
# endif()
