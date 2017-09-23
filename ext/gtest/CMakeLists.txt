cmake_minimum_required(VERSION 3.2)
project(gtest-builder)

find_package(Git REQUIRED)

include(ExternalProject)
ExternalProject_Add(googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           master
    CMAKE_ARGS -DBUILD_GTEST=ON
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    # disable install step
    INSTALL_COMMAND ""
)

# don't build googletest by default
set_target_properties(googletest PROPERTIES EXCLUDE_FROM_ALL TRUE)

# tell the main CMakeLists.txt where the gtest directories are
ExternalProject_Get_Property(googletest source_dir)
set(GTEST_INCLUDE_DIR ${source_dir}/googletest/include PARENT_SCOPE)
ExternalProject_Get_Property(googletest binary_dir)
set(GTEST_LIBS_DIR ${binary_dir}/googlemock/gtest PARENT_SCOPE)
