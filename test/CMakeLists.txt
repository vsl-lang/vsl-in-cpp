cmake_minimum_required(VERSION 3.2)

find_package(Threads REQUIRED)

include_directories(${GTEST_INCLUDE_DIR})
file(GLOB TEST_SOURCES ${VSL_TEST_DIR}/*.cpp)

add_executable(vsl-test EXCLUDE_FROM_ALL ${TEST_SOURCES})
add_dependencies(vsl-test googletest libvsl)
target_link_libraries(vsl-test ${GTEST_LIBS_DIR}/libgtest.a libvsl
    Threads::Threads)

add_custom_target(check COMMAND vsl-test)
