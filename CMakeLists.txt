cmake_minimum_required(VERSION 3.20)

project(Xenith
    VERSION 0.1.0
    DESCRIPTION "Sovereign Mathematical Optimization Solver Engine"
    LANGUAGES CXX
)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for language servers / tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Option toggles
option(XENITH_BUILD_TESTS "Build Xenith unit and integration tests" ON)
option(XENITH_BUILD_EXAMPLES "Build Xenith usage examples" ON)
option(XENITH_BUILD_TOOLS "Build Xenith inspection and benchmark tools" ON)
option(XENITH_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler flag configurations
if(MSVC)
    add_compile_options(/W4 /permissive-)
    if(XENITH_ENABLE_WARNINGS_AS_ERRORS)
        add_compile_options(/WX)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(XENITH_ENABLE_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Xenith Core Library (Foundation interface target for Phase 0)
add_library(xenith_core INTERFACE)
target_include_directories(xenith_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(xenith_core INTERFACE cxx_std_20)

# Include Subdirectories when implementation components are introduced in future phases
# add_subdirectory(src/xenith)

if(XENITH_BUILD_TESTS)
    enable_testing()
    # add_subdirectory(tests)
endif()

if(XENITH_BUILD_EXAMPLES)
    # add_subdirectory(examples)
endif()

if(XENITH_BUILD_TOOLS)
    # add_subdirectory(tools)
endif()

message(STATUS "Xenith Engine configured successfully (C++20).")
