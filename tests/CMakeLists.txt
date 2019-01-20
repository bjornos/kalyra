# windows:
# cmake -G "MSYS Makefiles" .
# make test

# project(UnitTest)

find_package(GTest REQUIRED)

# Unit Tests
set(UNIT_TEST_DIR unit)

include_directories(
    ${GTEST_INCLUDE_DIRS}
    ../${SRC_DIR}
)

add_executable(testInputParser ${UNIT_TEST_DIR}/testInputParser.cc)
add_executable(testFirmwareRelease ${UNIT_TEST_DIR}/testFirmwareRelease.cc)

target_link_libraries(testInputParser ${GTEST_BOTH_LIBRARIES})
target_link_libraries(testFirmwareRelease ${GTEST_BOTH_LIBRARIES} firmwareRelease releaseComponent packageRecipe cJSON)

add_test(InputParser testInputParser)
add_test(FirmwareRelease testFirmwareRelease)