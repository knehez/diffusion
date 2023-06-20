cmake_minimum_required(VERSION 3.5)


set(VCPKG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg" CACHE PATH "vcpkg folder")
set(VCPKG_BRANCH "master" CACHE STRING "vcpkg branch to checkout if not present")

find_package(Git QUIET)
if(NOT EXISTS "${VCPKG_PATH}")
    option(GIT_SUBMODULE "Check submodules during build" ON)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} clone -b "${VCPKG_BRANCH}" --single-branch  https://github.com/microsoft/vcpkg "${VCPKG_PATH}"
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        )
endif()

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(opencl-diffusion VERSION 1.0
             DESCRIPTION "Diff eq solver"
             LANGUAGES CXX)

find_package(Threads)
find_package(OpenMP)
find_package(OpenCL REQUIRED)

option(CodeCoverage "Code coverage" OFF)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# Add linker configuration
include(cmake/Linker.cmake)
configure_linker(project_options)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILE_FEATURES EHsc cxx_std_11)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules/)

set(CMAKE_POLICY_DEFAULT_CMP0025 NEW)

set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -O0 -g -fprofile-arcs -ftest-coverage ${CMAKE_CXX_FLAGS_DEBUG}")

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp:experimental")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "*** Please switch to 64 bit compiler! ***")
endif()

include_directories(src)

add_subdirectory(src)

if (CodeCoverage MATCHES ON)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        include(CodeCoverage)
        setup_target_for_coverage(${PROJECT_NAME}_coverage ${CMAKE_PROJECT_NAME}_test coverage)
        # setup_target_for_coverage_cobertura(${PROJECT_NAME}_cobertura poptests coverage --gtest_output=xml:coverage.junit.xml)
    endif() 
endif()
