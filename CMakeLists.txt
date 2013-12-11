cmake_minimum_required(VERSION 2.8)

project(craft)

add_executable(
    craft main.c client.c cube.c db.c map.c matrix.c noise.c util.c world.c
    sqlite/sqlite3.c lodepng/lodepng.c tinycthread/tinycthread.c)

if (MSVC)
    add_definitions(-W3 -MD -DWIN32 -D_CRT_SECURE_NO_WARNINGS)
else()
    add_definitions(-std=c99 -O3)
endif()

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(sqlite)
include_directories(lodepng)
include_directories(tinycthread)

if (APPLE)
    target_link_libraries(craft glfw ${GLFW_LIBRARIES})
elseif (WIN32)
    set(glew_INSTALL_ROOT glew-1.10.0)
    set(CMAKE_PREFIX_PATH ${glew_INSTALL_ROOT})
    set(CMAKE_LIBRARY_PATH ${glew_INSTALL_ROOT}/lib/Release/Win32)
    find_package(GLEW REQUIRED)
    include_directories(${GLEW_INCLUDE_DIRS})
    target_link_libraries(craft glfw ws2_32 ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})

    add_custom_command(TARGET craft POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       shaders $<TARGET_FILE_DIR:craft>/shaders)
    add_custom_command(TARGET craft POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different
                       "${PROJECT_SOURCE_DIR}/texture.png"
                       "$<TARGET_FILE_DIR:craft>")
    add_custom_command(TARGET craft POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different
                       "${glew_INSTALL_ROOT}/bin/Release/Win32/glew32.dll"
                       "$<TARGET_FILE_DIR:craft>")
else()
    find_package(GLEW REQUIRED)
    target_link_libraries(craft glfw dl ${GLEW_LIBRARIES} ${GLFW_LIBRARIES})
endif()
