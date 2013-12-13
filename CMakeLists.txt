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

# GLEW not known before cmake 2.8.10
FIND_PATH( GLEW_INCLUDE_PATH 
    NAMES GL/glew.h
    PATHS
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    ${GLEW_ROOT_DIR}/include
    DOC "The directory where GL/glew.h resides")

FIND_LIBRARY( GLEW_LIBRARY
    NAMES GLEW
    PATHS
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    ${GLEW_ROOT_DIR}/lib
    DOC "The GLEW library")
SET(GLEW_FOUND "NO")
IF (GLEW_INCLUDE_PATH AND GLEW_LIBRARY)
    SET(GLEW_LIBRARIES ${GLEW_LIBRARY})
    SET(GLEW_FOUND "YES")
    MESSAGE(STATUS "GLEW found")
ELSE(GLEW_INCLUDE_PATH AND GLEW_LIBRARY)
    MESSAGE(FATAL_ERROR "GLEW not found
    You may install devel and library packages")
ENDIF (GLEW_INCLUDE_PATH AND GLEW_LIBRARY)

if (APPLE)
    target_link_libraries(craft glfw ${GLFW_LIBRARIES})
else()
    target_link_libraries(craft glfw dl ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})
endif()
