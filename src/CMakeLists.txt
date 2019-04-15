# windows:
# cmake -G "MSYS Makefiles" .

if (WIN32)
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()

add_library(manifest manifest.cc)
add_library(firmwareRelease firmwareRelease.cc)
add_library(packageRecipe packageRecipe.cc)
add_library(scriptGenerator scriptGenerator.cc)
add_library(releaseComponent releaseComponent.cc)
add_library(inputParser inputParser.cc)
add_library(cJSON ../${JSON_PATH}/cJSON.c)

# Firmware Factory
add_executable(ff ff.cc)

if (UNIX)
target_link_libraries (ff
	manifest
	firmwareRelease
	packageRecipe
	scriptGenerator
	releaseComponent
	inputParser
	cJSON
	stdc++fs
)
else()
target_link_libraries (ff
	manifest
	firmwareRelease
	packageRecipe
	scriptGenerator
	releaseComponent
	inputParser
	cJSON
)
endif()

# Recipe Generator
add_executable(rpgen rpgen.cc)

target_link_libraries(rpgen packageRecipe inputParser cJSON)