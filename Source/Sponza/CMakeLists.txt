# ============================================================================
# Sponza Example Application
# ============================================================================
cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

# ============================================================================
# Project Definition
# ============================================================================
if(NOT PROJECT_NAME STREQUAL "Fidelity")
    project(Sponza
        VERSION 1.0.0
        DESCRIPTION "Sponza Example Application"
        LANGUAGES CXX
    )
endif()

# ============================================================================
# Create Executable using Fidelity utilities
# ============================================================================

fidelity_add_executable(Sponza
    COPY_RESOURCES
)

# ============================================================================
# Additional Configuration
# ============================================================================

# Apply common build configurations
fidelity_set_build_config(Sponza)

# Add compiler warnings if enabled
if(FIDELITY_ENABLE_WARNINGS)
    fidelity_add_warnings(Sponza)
endif()

message(STATUS "Sponza example application configured")