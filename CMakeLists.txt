cmake_minimum_required(VERSION 3.9)
project(VSL)

find_package(LLVM REQUIRED CONFIG)
find_package(Doxygen)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# require c++14 with warnings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
# vsl specific directories
set(VSL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(VSL_DOCS_DIR ${PROJECT_SOURCE_DIR}/docs)

option(BUILD_DOCS "Build documentation using Doxygen" ${DOXYGEN_FOUND})

if(BUILD_DOCS)
    # make sure doxygen is there
    if (NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()
    set(doxyfile ${PROJECT_SOURCE_DIR}/Doxyfile)
    # create the build target
    add_custom_target(docs
        # create the docs directory
        COMMAND ${CMAKE_COMMAND} -E make_directory ${VSL_DOCS_DIR}
        # do the doxygen command
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Building documentation using Doxygen"
        VERBATIM)
endif()

# set the default build type to debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# setup includes/definitions and source files
include_directories(${VSL_SOURCE_DIR})
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
file(GLOB SOURCES "${VSL_SOURCE_DIR}/*.cpp")

# build the executable
add_executable(vsl ${SOURCES})

# find the necessary llvm libs and link them with the executable
llvm_map_components_to_libnames(llvm_libs core)
target_link_libraries(vsl ${llvm_libs})
