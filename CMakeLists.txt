cmake_minimum_required(VERSION 3.1.0)
project(Kalyra)

set(CMAKE_CXX_STANDARD 11)

set(JSON_PATH lib/cJSON)
set(SOURCE_DIR src)

include_directories(lib)
include_directories(src)

link_directories(lib/cJSON)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

