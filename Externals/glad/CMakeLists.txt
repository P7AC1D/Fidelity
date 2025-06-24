# ============================================================================
# GLAD - OpenGL Loader Library
# ============================================================================
cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# ============================================================================
# Project Definition
# ============================================================================
if(NOT PROJECT_NAME STREQUAL "Fidelity")
    project(glad
        VERSION 1.0.0
        DESCRIPTION "GLAD OpenGL Loader"
        LANGUAGES C
    )
endif()

# ============================================================================
# Library Definition
# ============================================================================

# Create GLAD library
add_library(glad STATIC
    include/glad/glad.h
    src/glad.c
)

# Add alias for modern CMake usage
add_library(glad::glad ALIAS glad)

# ============================================================================
# Target Properties
# ============================================================================

set_target_properties(glad PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ============================================================================
# Include Directories
# ============================================================================

target_include_directories(glad
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ============================================================================
# Platform-specific Configuration
# ============================================================================

if(WIN32)
    target_compile_definitions(glad PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

message(STATUS "GLAD library configured successfully")