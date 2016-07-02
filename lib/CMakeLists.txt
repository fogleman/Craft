cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(konstructs-lib)

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

if(MINGW)
  add_definitions(-DWIN32)
  add_definitions(-DEIGEN_DONT_ALIGN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
endif()

set(CMAKE_BUILD_TYPE Debug)

find_package(ZLIB REQUIRED)

include_directories(
  ${ZLIB_INCLUDE_DIRS}
  ../dependencies/nanogui/ext/glfw/include
  ../dependencies/nanogui/ext/glew/include
  ../dependencies/nanogui/ext/eigen
  ../dependencies/lodepng
  ../dependencies/optional-lite
  ../dependencies/tinyobjloader
  include
  )

FILE(
  GLOB SOURCE_FILES
  src/*.cpp
  ../dependencies/lodepng/lodepng.cpp
  ../dependencies/tinyobjloader/tiny_obj_loader.cc)

add_library(konstructs-lib ${SOURCE_FILES})
