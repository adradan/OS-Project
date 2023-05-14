cmake_minimum_required(VERSION 3.22)
project(SimOS_lib)

set(CMAKE_CXX_STANDARD 17)

#include(FetchContent)
#FetchContent_Declare(
#        googletest
#        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
#)
## For Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#FetchContent_MakeAvailable(googletest)

#add_subdirectory(lib)
#include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})


add_library(SimOS_lib STATIC SimOS.cpp SimOS.h tests.cpp DiskQueue.cpp DiskQueue.h)
target_link_libraries(SimOS_lib gtest gtest_main)
