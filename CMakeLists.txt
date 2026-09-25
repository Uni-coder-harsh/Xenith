cmake_minimum_required(VERSION 3.20)

project(Xenith
    VERSION 0.2.0
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
option(XENITH_BUILD_EXAMPLES "Build Xenith usage examples" OFF)
option(XENITH_BUILD_TOOLS "Build Xenith inspection and benchmark tools" OFF)
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

# Xenith Library Target
add_library(xenith_lib STATIC
    src/xenith/numerics/vector_ops.cpp
    src/xenith/numerics/sparse_matrix.cpp
    src/xenith/numerics/scaling.cpp
    src/xenith/numerics/lu_factorization.cpp
    src/xenith/model/canonical_model.cpp
    src/xenith/model/model_validator.cpp
    src/xenith/io/mps/mps_reader.cpp
    src/xenith/solver/common/basis_manager.cpp
    src/xenith/solver/lp/revised_simplex_solver.cpp
    src/xenith/solver/lp/pdlp_solver.cpp
    src/xenith/solution/lp_solution_validator.cpp
    src/xenith/presolve/presolver.cpp
    src/xenith/ui/terminal_ui.cpp
)

target_include_directories(xenith_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(xenith_lib PUBLIC cxx_std_20)

# Interface target alias for architectural backward compatibility
add_library(xenith_core INTERFACE)
target_link_libraries(xenith_core INTERFACE xenith_lib)

# User-facing MPS Inspector CLI Executable
add_executable(xenith_mps tools/model_inspector/main.cpp)
target_link_libraries(xenith_mps PRIVATE xenith_lib)
set_target_properties(xenith_mps PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_custom_command(TARGET xenith_mps POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:xenith_mps>"
    "${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:xenith_mps>"
    COMMENT "Copying xenith_mps executable to bin/ directory"
)

if(XENITH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "Xenith Engine configured successfully (C++20). Target: xenith_lib, Executable: xenith_mps")

