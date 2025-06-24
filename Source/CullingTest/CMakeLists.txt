# ============================================================================
# CullingTest Example Application
# ============================================================================
cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# ============================================================================
# Project Definition
# ============================================================================
if(NOT PROJECT_NAME STREQUAL "Fidelity")
    project(CullingTest
        VERSION 1.0.0
        DESCRIPTION "CullingTest Example Application"
        LANGUAGES CXX
    )
endif()

# ============================================================================
# Create Executable using Fidelity utilities
# ============================================================================

fidelity_add_executable(CullingTest
    COPY_RESOURCES
)

# ============================================================================
# Additional Configuration
# ============================================================================

# Apply common build configurations
fidelity_set_build_config(CullingTest)

# Add compiler warnings if enabled
if(FIDELITY_ENABLE_WARNINGS)
    fidelity_add_warnings(CullingTest)
endif()

message(STATUS "CullingTest example application configured")