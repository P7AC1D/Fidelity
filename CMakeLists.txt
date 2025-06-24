# ============================================================================
# Fidelity Engine - Modern CMake Configuration
# ============================================================================
cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# Set modern CMake policies
cmake_policy(VERSION 3.21)
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW) # option() honors normal variables
endif()
if(POLICY CMP0175)
    cmake_policy(SET CMP0175 NEW) # add_custom_command() rejects invalid arguments
endif()

# Handle compatibility with external dependencies that may use older CMake versions
# This allows GLM and other external libraries to work with modern CMake
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Project definition with versioning
project(Fidelity 
    VERSION 1.0.0
    DESCRIPTION "Fidelity Engine - Modern Graphics Engine"
    LANGUAGES CXX C
)

# ============================================================================
# CMake Modules
# ============================================================================

# Add our custom CMake utilities
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FidelityUtils)

# ============================================================================
# Build Configuration
# ============================================================================

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type validation
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# ============================================================================
# Options and Settings
# ============================================================================

option(FIDELITY_BUILD_TESTS "Build test executables" ON)
option(FIDELITY_BUILD_EXAMPLES "Build example applications" ON)
option(FIDELITY_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(FIDELITY_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# External library configuration
set(ENTITYX_BUILD_SHARED FALSE CACHE BOOL "Build EntityX as shared library")
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "Treat Assimp warnings as errors")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build Assimp tests")
set(ASSIMP_INSTALL OFF CACHE BOOL "Install Assimp")

# ============================================================================
# Compiler Configuration
# ============================================================================

# Compiler-specific settings
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # MSVC specific settings
    add_compile_options(/W4)
    if(FIDELITY_WARNINGS_AS_ERRORS)
        add_compile_options(/WX)
    endif()
    # Enable parallel compilation
    add_compile_options(/MP)
    # Enable latest C++ features
    add_compile_options(/permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang specific settings
    if(FIDELITY_ENABLE_WARNINGS)
        add_compile_options(-Wall -Wextra -Wpedantic)
    endif()
    if(FIDELITY_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
endif()

# ============================================================================
# Dependency Management
# ============================================================================

# Find required packages with proper error handling
find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
    message(FATAL_ERROR "OpenGL not found. Please install OpenGL development libraries.")
endif()

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found. Please install Vulkan SDK.")
endif()

message(STATUS "Found OpenGL: ${OPENGL_VERSION_STRING}")
message(STATUS "Found Vulkan: ${Vulkan_VERSION}")

# ============================================================================
# External Libraries
# ============================================================================

message(STATUS "Configuring external dependencies...")

# GLFW
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Externals/glfw/CMakeLists.txt")
    add_subdirectory(Externals/glfw)
    message(STATUS "✓ GLFW configured")
else()
    message(FATAL_ERROR "GLFW not found in Externals/glfw")
endif()

# Assimp
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Externals/assimp/CMakeLists.txt")
    add_subdirectory(Externals/assimp)
    message(STATUS "✓ Assimp configured")
else()
    message(FATAL_ERROR "Assimp not found in Externals/assimp")
endif()

# GLAD
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Externals/glad/CMakeLists.txt")
    add_subdirectory(Externals/glad)
    message(STATUS "✓ GLAD configured")
else()
    message(FATAL_ERROR "GLAD not found in Externals/glad")
endif()

# GLM - Handle version compatibility with external submodule
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Externals/glm/CMakeLists.txt")
    # Save current policies and set compatibility mode for GLM
    cmake_policy(PUSH)
    # Allow GLM to use its older CMake minimum version
    set(CMAKE_POLICY_DEFAULT_CMP0048 OLD)
    # Add GLM subdirectory with suppressed warnings
    add_subdirectory(Externals/glm EXCLUDE_FROM_ALL)
    # Restore our policies
    cmake_policy(POP)
    message(STATUS "✓ GLM configured (compatibility mode)")
else()
    message(FATAL_ERROR "GLM not found in Externals/glm")
endif()

# ============================================================================
# Engine Components
# ============================================================================

message(STATUS "Configuring engine components...")

# Engine core library
add_subdirectory(Source/Engine)
message(STATUS "✓ Engine core configured")

# Test applications
if(FIDELITY_BUILD_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/Tests/CMakeLists.txt")
        add_subdirectory(Source/Tests)
        message(STATUS "✓ Tests configured")
    endif()
endif()

# Example applications
if(FIDELITY_BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/Test3D/CMakeLists.txt")
        add_subdirectory(Source/Test3D)
        message(STATUS "✓ Test3D example configured")
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sponza/CMakeLists.txt")
        add_subdirectory(Source/Sponza)
        message(STATUS "✓ Sponza example configured")
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/CullingTest/CMakeLists.txt")
        add_subdirectory(Source/CullingTest)
        message(STATUS "✓ CullingTest example configured")
    endif()
endif()

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "===============================================")
message(STATUS "Fidelity Engine Configuration Summary")
message(STATUS "===============================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build tests: ${FIDELITY_BUILD_TESTS}")
message(STATUS "Build examples: ${FIDELITY_BUILD_EXAMPLES}")
message(STATUS "Warnings enabled: ${FIDELITY_ENABLE_WARNINGS}")
message(STATUS "Warnings as errors: ${FIDELITY_WARNINGS_AS_ERRORS}")
message(STATUS "===============================================")
message(STATUS "")