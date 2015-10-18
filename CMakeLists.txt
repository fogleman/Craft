cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(konstructs)

#-----------------------------------------------------------------------
# Check for C++11 enabled compiler
#-----------------------------------------------------------------------

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#-----------------------------------------------------------------------
# Nano GUI
#-----------------------------------------------------------------------
option(NANOGUI_BUILD_PYTHON OFF)
option(NANOGUI_BUILD_EXAMPLE OFF)
add_subdirectory(dependencies/nanogui)

#-----------------------------------------------------------------------
# Misc
#-----------------------------------------------------------------------

set(CMAKE_BUILD_TYPE Debug)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS} dependencies/nanogui/include/ dependencies/nanogui/ext/glfw/include dependencies/lodepng)

FILE(GLOB SOURCE_FILES src/*.cpp dependencies/lodepng/lodepng.cpp src/noise/*.cpp src/tinycthread/*.cpp)
add_executable(konstructs ${SOURCE_FILES})

if(MINGW)
  target_link_libraries(konstructs ws2_32.lib
    ${ZLIB_LIBRARIES} nanogui)
else()
  target_link_libraries(konstructs
    ${ZLIB_LIBRARIES} nanogui glfw)
endif()
