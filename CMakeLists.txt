# windows:
# cmake -G "MSYS Makefiles" .

cmake_minimum_required(VERSION 3.1.0)
project(Kalyra)

set(CMAKE_SOURCE_DIR src)
set(CMAKE_CXX_STANDARD 11)

set(SOURCES
${CMAKE_SOURCE_DIR}/main.cc
${CMAKE_SOURCE_DIR}/manifest.cc
${CMAKE_SOURCE_DIR}/firmwareRelease.cc
${CMAKE_SOURCE_DIR}/packageRecipe.cc
${CMAKE_SOURCE_DIR}/scriptGenerator.cc
${CMAKE_SOURCE_DIR}/releaseComponent.cc
lib/cJSON/cJSON.c)

include_directories(lib)

# Firmware Factory
add_executable(ff ${SOURCES})
