# ============================================================================
# Fidelity Engine - Core Library
# ============================================================================
cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# ============================================================================
# Project Definition
# ============================================================================
if(NOT PROJECT_NAME STREQUAL "Fidelity")
    project(FidelityEngine
        VERSION 1.0.0
        DESCRIPTION "Fidelity Engine Core Library"
        LANGUAGES CXX
    )
endif()

# ============================================================================
# Source File Management
# ============================================================================

# Collect source files explicitly (avoid GLOB for better build dependency tracking)
set(ENGINE_HEADERS "")
set(ENGINE_SOURCES "")

# Find all header files
file(GLOB_RECURSE ENGINE_HEADERS_TEMP
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hxx"
)

# Find all source files
file(GLOB_RECURSE ENGINE_SOURCES_TEMP
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx" 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Filter out any test files or temporary files
foreach(file ${ENGINE_HEADERS_TEMP})
    if(NOT file MATCHES "test|Test|TEST|temp|tmp")
        list(APPEND ENGINE_HEADERS ${file})
    endif()
endforeach()

foreach(file ${ENGINE_SOURCES_TEMP})
    if(NOT file MATCHES "test|Test|TEST|temp|tmp")
        list(APPEND ENGINE_SOURCES ${file})
    endif()
endforeach()

# Ensure we have source files
if(NOT ENGINE_SOURCES AND NOT ENGINE_HEADERS)
    message(FATAL_ERROR "No source files found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

list(LENGTH ENGINE_SOURCES NUM_SOURCES)
list(LENGTH ENGINE_HEADERS NUM_HEADERS)
message(STATUS "Engine: Found ${NUM_SOURCES} source files and ${NUM_HEADERS} header files")

# ============================================================================
# Library Definition
# ============================================================================

# Create the engine library
add_library(engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Add alias for modern CMake usage
add_library(Fidelity::Engine ALIAS engine)

# ============================================================================
# Target Properties
# ============================================================================

# Set target properties
set_target_properties(engine PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Platform-specific settings
if(WIN32)
    set_target_properties(engine PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

# ============================================================================
# Include Directories
# ============================================================================

# Public include directories (interface requirements)
target_include_directories(engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Private include directories for external dependencies
target_include_directories(engine PRIVATE
    "${CMAKE_SOURCE_DIR}/Externals/glfw/include"
    "${CMAKE_SOURCE_DIR}/Externals/assimp/include"
    "${CMAKE_SOURCE_DIR}/Externals/glad/include"
    "${CMAKE_SOURCE_DIR}/Externals/glm"
)

# ============================================================================
# Dependencies and Linking
# ============================================================================

# Find required packages
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

# Link libraries using modern target-based approach
target_link_libraries(engine
    PUBLIC
        OpenGL::GL
        Vulkan::Vulkan
        glad  # Make GLAD available to consumers
    PRIVATE
        glfw
        assimp
        glm
)

# ============================================================================
# Compiler Definitions
# ============================================================================

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(engine PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
endif()

# Build type definitions
target_compile_definitions(engine PRIVATE
    $<$<CONFIG:Debug>:DEBUG _DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Graphics API definitions (you might want to make these configurable)
target_compile_definitions(engine PUBLIC
    FIDELITY_OPENGL_SUPPORT
    FIDELITY_VULKAN_SUPPORT
)

# ============================================================================
# Compiler Options
# ============================================================================

# Compiler-specific options
if(MSVC)
    target_compile_options(engine PRIVATE
        /W4                    # Warning level 4
        /permissive-           # Disable non-conforming code
        /Zc:__cplusplus        # Enable correct __cplusplus macro
        /Zc:preprocessor       # Enable conforming preprocessor
        $<$<CONFIG:Release>:/O2 /Ob2>  # Optimize for speed in Release
        $<$<CONFIG:Debug>:/Od /Zi>     # No optimization, debug info in Debug
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(engine PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Debug>:-g -O0>
    )
endif()

# ============================================================================
# IDE Organization
# ============================================================================

# Organize source files in IDE
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# ============================================================================
# Installation (Optional)
# ============================================================================

# Install targets (uncomment if needed)
# install(TARGETS engine
#     EXPORT FidelityEngineTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )

# install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
#     DESTINATION include/FidelityEngine
#     FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
# )

message(STATUS "Engine library configured successfully")