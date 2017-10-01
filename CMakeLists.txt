cmake_minimum_required(VERSION 3.2)
project(VSL)

find_package(LLVM 5.0.0 REQUIRED CONFIG)
find_package(Doxygen)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# require c++14 with warnings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wno-unused-parameter")

# vsl specific directories
set(VSL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(VSL_DOCS_DIR ${PROJECT_SOURCE_DIR}/docs)
set(VSL_TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set(VSL_EXT_PROJECTS_DIR ${PROJECT_SOURCE_DIR}/ext)

option(VSL_BUILD_DOCS "Build documentation using Doxygen" ${DOXYGEN_FOUND})
option(VSL_INCLUDE_TESTS "Allow building the test code through the check target"
    OFF)

if(VSL_BUILD_DOCS)
    # make sure doxygen is there
    if (NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()
    set(doxyfile ${PROJECT_SOURCE_DIR}/Doxyfile)
    # add the docs target
    add_custom_target(docs
        COMMAND ${CMAKE_COMMAND} -E make_directory ${VSL_DOCS_DIR}
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Building documentation using Doxygen"
        VERBATIM)
endif()

# set the default build type to debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# config so the compiler knows what files are where
include_directories(${VSL_SOURCE_DIR} ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
set(VSL_MAIN_CPP ${VSL_SOURCE_DIR}/main.cpp)
file(GLOB SOURCES ${VSL_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM SOURCES ${VSL_MAIN_CPP})

# used for other parts of vsl that don't use src/main.cpp, e.g. the test code
add_library(libvsl STATIC ${SOURCES})
set_target_properties(libvsl PROPERTIES PREFIX "") # so i don't get liblibvsl.a
llvm_map_components_to_libnames(llvm_libs all)
target_link_libraries(libvsl ${llvm_libs})

if(VSL_INCLUDE_TESTS)
    add_subdirectory(${VSL_EXT_PROJECTS_DIR}/gtest)
    add_subdirectory(${VSL_TEST_DIR})
endif()

# build the executable (with main.cpp)
add_executable(vsl ${VSL_MAIN_CPP})

# link the executable with the required libraries
add_dependencies(vsl libvsl)
target_link_libraries(vsl libvsl)
