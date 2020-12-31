cmake_minimum_required(VERSION 3.16)
project(turbo_brainfuck VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(main src/main.cpp)

set(CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING "Flags used by the CXX compiler during DEBUG builds" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "Flags used by the CXX compiler during RELEASE builds" FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address" CACHE STRING "Flags used by the CXX compiler during ASAN builds" FORCE)

set(allowableBuildTypes Debug Release Asan)

if (NOT CMAKE_BUILD_TYPE IN_LIST allowableBuildTypes)
    message(FATAL_ERROR "${CMAKE_BUILD_TYPE} is not a defined build type")
endif()