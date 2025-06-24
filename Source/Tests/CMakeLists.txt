# ============================================================================
# Fidelity Engine Tests
# ============================================================================
cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# ============================================================================
# Project Definition
# ============================================================================
if(NOT PROJECT_NAME STREQUAL "Fidelity")
    project(FidelityTests
        VERSION 1.0.0
        DESCRIPTION "Fidelity Engine Unit Tests"
        LANGUAGES CXX
    )
endif()

# ============================================================================
# Test Configuration
# ============================================================================

# Collect test source files
file(GLOB TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
)

file(GLOB TEST_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Ensure we have test files
if(NOT TEST_SOURCES)
    message(WARNING "No test source files found in ${CMAKE_CURRENT_SOURCE_DIR}")
    return()
endif()

# Create test executable
add_executable(Tests ${TEST_SOURCES} ${TEST_HEADERS})

# Set target properties
set_target_properties(Tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ============================================================================
# Dependencies
# ============================================================================

# Link with engine and dependencies
target_link_libraries(Tests PRIVATE
    engine
    glm
)

# Include directories
target_include_directories(Tests PRIVATE
    "${CMAKE_SOURCE_DIR}/Source/Engine"
    "${CMAKE_SOURCE_DIR}/Externals/glm"
)

# ============================================================================
# Platform Configuration
# ============================================================================

if(WIN32)
    target_compile_definitions(Tests PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
endif()

# Build configurations
target_compile_definitions(Tests PRIVATE
    $<$<CONFIG:Debug>:DEBUG _DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# ============================================================================
# IDE Organization
# ============================================================================

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES} ${TEST_HEADERS})

# ============================================================================
# Test Integration (Optional - for CTest)
# ============================================================================

# Enable testing if CTest is available
if(CMAKE_TESTING_ENABLED OR BUILD_TESTING)
    enable_testing()
    add_test(NAME FidelityTests COMMAND Tests)
    set_tests_properties(FidelityTests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

message(STATUS "Test executable configured successfully")