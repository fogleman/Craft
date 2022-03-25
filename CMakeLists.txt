cmake_minimum_required(VERSION 3.16.3)

###############################################################################
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-g -O0 -std=c++11 -U__STRICT_ANSI__ -fprofile-instr-generate -fcoverage-mapping")


project(craft)

FILE(GLOB SOURCE_FILES src/*.c)
include_directories(googletest/include) # this is so we can #include <gtest/gtest.h>
#include_directories(include)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

###############################################################################
add_executable(tests) 
target_include_directories(tests PUBLIC model)
target_sources(tests PUBLIC 
	${SOURCE_FILES}
	tests/tests.cpp
	src/deleteme.c
	deps/glew/src/glew.c
        deps/lodepng/lodepng.c
        deps/noise/noise.c
        deps/sqlite/sqlite3.c
        deps/tinycthread/tinycthread.c)

target_link_libraries(tests PRIVATE gtest) # link google test to this executable
set_property(TARGET tests PROPERTY CXX_STANDARD 11)

###############################################################################
add_custom_target(coverage
	COMMAND ln -f -s ../textures .
		COMMAND ln -f -s ../shaders .
		COMMAND ./tests
		    COMMAND llvm-profdata merge -sparse default.profraw -o cumulative.profdata
		        COMMAND llvm-cov show -ignore-filename-regex='../google*/*' -format="html" ./tests -instr-profile=cumulative.profdata > coverageLines.html
			    COMMAND llvm-cov report -show-region-summary=false -ignore-filename-regex='../google*/*'  ./tests -instr-profile=cumulative.profdata > coverageSummary.txt
			        COMMAND cat coverageSummary.txt
				    COMMAND cd build
				    )
			    add_dependencies(coverage tests)


add_executable(
    craft
    ${SOURCE_FILES}
    tests/craft_main.c
    deps/glew/src/glew.c
    deps/lodepng/lodepng.c
    deps/noise/noise.c
    deps/sqlite/sqlite3.c
    deps/tinycthread/tinycthread.c)

add_definitions(-std=c99 -O3)

add_subdirectory(deps/glfw)
include_directories(deps/glew/include)
include_directories(deps/glfw/include)
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

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

if(APPLE)
    target_link_libraries(craft glfw
        ${GLFW_LIBRARIES} ${CURL_LIBRARIES})
endif()

if(UNIX)
	target_link_libraries(craft PUBLIC dl PUBLIC glfw
        ${GLFW_LIBRARIES} ${CURL_LIBRARIES})
endif()

if(MINGW)
    target_link_libraries(craft ws2_32.lib glfw
        ${GLFW_LIBRARIES} ${CURL_LIBRARIES})
endif()

if(UNIX)
	target_link_libraries(tests PUBLIC dl PUBLIC glfw
        ${GLFW_LIBRARIES} ${CURL_LIBRARIES})
endif()

