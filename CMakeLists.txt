cmake_minimum_required(VERSION 2.8.8)
project(NuSMV-p)

set(CMAKE_C_STANDARD 99)

set(NUSMV_SOURCE_DIR "${PROJECT_SOURCE_DIR}/NuSMV"
        CACHE_PATH "Source directory of NuSMV")
set(NUSMV_BUILD_DIR "${PROJECT_BINARY_DIR}/build-nusmv"
        CACHE_PATH "Build directory of NuSMV")

add_subdirectory(NuSMV)
add_library(TER library.c library.h)