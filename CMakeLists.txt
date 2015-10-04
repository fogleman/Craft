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
# CPM configuration
#-----------------------------------------------------------------------

set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm_packages" CACHE TYPE STRING)
find_package(Git)
if(NOT GIT_FOUND)
message(FATAL_ERROR "CPM requires Git.")
endif()
if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
message(STATUS "Cloning repo (https://github.com/iauns/cpm)")
execute_process(
	COMMAND "${GIT_EXECUTABLE}" clone https://github.com/iauns/cpm ${CPM_DIR}
	RESULT_VARIABLE error_code
	OUTPUT_QUIET ERROR_QUIET)
if(error_code)
	message(FATAL_ERROR "CPM failed to get the hash for HEAD")
endif()
endif()
include(${CPM_DIR}/CPM.cmake)

#------------------------------------------------------------------------------
# CPM Modules
#------------------------------------------------------------------------------

CPM_AddModule("cpm-glfw"
	GIT_REPOSITORY "https://github.com/nsg/cpm-glfw.git"
	GIT_TAG "27d3dd4a8fd34f50f2e4cf1583484e1c4ac4bcec")

CPM_AddModule("cpm-glew"
	GIT_REPOSITORY "https://github.com/iauns/cpm-glew.git"
	GIT_TAG "1.0.1")

CPM_AddModule("cpm-lodepng"
	GIT_REPOSITORY "https://github.com/nsg/cpm-lodepng.git"
	GIT_TAG "9879d938ce3f110ccca020979d282ea2e4713398")

CPM_Finish()

#-----------------------------------------------------------------------
# Misc
#-----------------------------------------------------------------------

set(CMAKE_BUILD_TYPE Debug)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

FILE(GLOB SOURCE_FILES src/*.cpp src/noise/*.cpp src/tinycthread/*.cpp)
add_executable(konstructs ${SOURCE_FILES})
target_link_libraries(konstructs ${CPM_LIBRARIES} ${ZLIB_LIBRARIES})
