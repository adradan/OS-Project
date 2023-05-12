cmake_minimum_required(VERSION 3.22)
project(SimOS)

set(CMAKE_CXX_STANDARD 17)

#add_library(SimOS_lib SimOS_lib/SimOS.cpp)

add_subdirectory(SimOS_lib)
add_subdirectory(SimOS_tests)

include_directories(SimOS_lib)

target_link_libraries(SimOS_lib)

target_include_directories(Google_Tests_run PUBLIC SimOS_lib)
#target_include_directories(SimOS_lib PUBLIC SimOS_tests)

