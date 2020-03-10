cmake_minimum_required(VERSION 3.12.0)

project(Kalyra)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()



# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")

# If you only include this third party in PRIVATE source files, you do not
# need to install it when your main project gets installed.
# set(JSON_Install OFF CACHE INTERNAL "")

# Don't use include(nlohmann_json/CMakeLists.txt) since that carries with it
# unintended consequences that will break the build.  It's generally
# discouraged (although not necessarily well documented as such) to use
# include(...) for pulling in other CMake projects anyways.

# add_subdirectory(lib/json)

include_directories(lib)
include_directories(include)


add_executable(kalyra src/kalyra.cc src/input_parser.cc src/release.cc src/script_generator.cc src/prod_conf.cc src/manifest.cc)

#add_executable(env_setup src/env_setup.cc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-deprecated")

# target_link_libraries(kalyra PRIVATE nlohmann_json::nlohmann_json)
# target_link_libraries(kalyra PRIVATE nlohmann_json::nlohmann_json)
