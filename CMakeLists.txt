# windows:
# cmake -G "MSYS Makefiles" .

cmake_minimum_required(VERSION 3.1.0)
project(Kalyra)

set(CMAKE_SOURCE_DIR src)
set(CMAKE_CXX_STANDARD 11)
set(SOURCES src/main.cc src/firmwareRelease.cc src/packageRecipe.cc src/scriptGenerator.cc src/releaseComponent.cc lib/cJSON/cJSON.c)

include_directories(lib)

# Firmware Factory
add_executable(ff ${SOURCES})
