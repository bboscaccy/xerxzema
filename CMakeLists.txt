cmake_minimum_required(VERSION 3.0)

project(xerxzema)

set(xerxzema_VERSION_MAJOR 0)
set(xerxzema_VERSION_MINOR 1)

enable_testing()
find_package(GTest REQUIRED)
find_package(LLVM 3.9 REQUIRED)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${GTEST_INCLUDE_DIRS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
include_directories(${LLVM_INCLUDE_DIRS})

add_subdirectory(lib)
