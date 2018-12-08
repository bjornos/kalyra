# windows:
# cmake -G "MSYS Makefiles" .

# project(UnitTest)

find_package(GTest REQUIRED)

# Unit Tests
set(UNIT_TEST_DIR unit)

include_directories(
    ${GTEST_INCLUDE_DIRS}
    ../${SRC_DIR}
)

add_executable(inputParser ${UNIT_TEST_DIR}/testInputParser.cc)

target_link_libraries(inputParser ${GTEST_BOTH_LIBRARIES})

add_test(InputParser inputParser)