cmake_minimum_required(VERSION 3.1.0)
project(Kalyra)

if (UNIX)
	set(CMAKE_CXX_STANDARD 17)
else()
	set(CMAKE_CXX_STANDARD 11)
endif()


#set(CMAKE_CXX_STANDARD_REQUIRED YES)
#set(CMAKE_CXX_EXTENSIONS OFF)


set(JSON_PATH lib/cJSON)
set(SOURCE_DIR src)

include_directories(lib)
include_directories(include)

link_directories(lib/cJSON)

add_subdirectory(src)

if (UNIX)
	enable_testing()
	add_subdirectory(tests)
endif()

