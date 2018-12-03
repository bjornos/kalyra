# windows:
# cmake -G "MSYS Makefiles" .

cmake_minimum_required(VERSION 3.1.0)
project(Kalyra)

set(CMAKE_SOURCE_DIR src)
set(CMAKE_CXX_STANDARD 11)

IF (WIN32)
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
ENDIF()

set(FIRMWARE_FACTORY_SOURCES
${CMAKE_SOURCE_DIR}/ff.cc
${CMAKE_SOURCE_DIR}/manifest.cc
${CMAKE_SOURCE_DIR}/firmwareRelease.cc
${CMAKE_SOURCE_DIR}/packageRecipe.cc
${CMAKE_SOURCE_DIR}/scriptGenerator.cc
${CMAKE_SOURCE_DIR}/releaseComponent.cc
lib/cJSON/cJSON.c)

set(RECIPE_GENERATOR_SOURCES
${CMAKE_SOURCE_DIR}/rpgen.cc
${CMAKE_SOURCE_DIR}/packageRecipe.cc
lib/cJSON/cJSON.c)

include_directories(lib)

# Firmware Factory
add_executable(ff ${FIRMWARE_FACTORY_SOURCES})
# Recipe Generator
add_executable(rpgen ${RECIPE_GENERATOR_SOURCES})
