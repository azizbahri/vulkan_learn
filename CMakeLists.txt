#cmake file for the project
cmake_minimum_required(VERSION 2.8)

#project name
project(vulkan_tutorial)

#set the c++ standard
set(CMAKE_CXX_STANDARD 17)

#set compiler flags including debug symbols and optimization level to 0
set(CMAKE_CXX_FLAGS "-g -O0")


#set the output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#set the include path
include_directories(${CMAKE_SOURCE_DIR}/include)

#set the source files for test
set(SOURCE_FILES_TEST src/test.cpp)

#set the source files for triangle tutorial
set(SOURCE_FILES_TRIANGLE src/triangle.cpp)

#set the executable for test
add_executable(test ${SOURCE_FILES_TEST})

#set the executable for triangle tutorial
add_executable(triangle ${SOURCE_FILES_TRIANGLE})

#create a variable for the libraries
set(LIBS vulkan glfw X11 Xxf86vm Xrandr pthread Xi dl)

#set the libraries for test
target_link_libraries(test ${LIBS})

#set the libraries for triangle tutorial
target_link_libraries(triangle ${LIBS})
