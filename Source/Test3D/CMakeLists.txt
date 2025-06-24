# ============================================================================
# Test3D Example Application
# ============================================================================
cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# ============================================================================
# Project Definition
# ============================================================================
if(NOT PROJECT_NAME STREQUAL "Fidelity")
    project(Test3D
        VERSION 1.0.0
        DESCRIPTION "Test3D Example Application"
        LANGUAGES CXX
    )
endif()

# ============================================================================
# Create Executable using Fidelity utilities
# ============================================================================

fidelity_add_executable(Test3D
    COPY_RESOURCES
)

# ============================================================================
# Additional Configuration
# ============================================================================

# Apply common build configurations
fidelity_set_build_config(Test3D)

# Add compiler warnings if enabled
if(FIDELITY_ENABLE_WARNINGS)
    fidelity_add_warnings(Test3D)
endif()

message(STATUS "Test3D example application configured")