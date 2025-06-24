# ============================================================================
# Fidelity Engine CMake Utilities
# ============================================================================

# Function to configure a Fidelity executable with common settings
function(fidelity_add_executable target_name)
    cmake_parse_arguments(FIDELITY
        "COPY_RESOURCES" # Options
        "SOURCE_DIR"     # Single value args
        "SOURCES;HEADERS;DEPENDENCIES" # Multi-value args
        ${ARGN}
    )

    # Default source directory
    if(NOT FIDELITY_SOURCE_DIR)
        set(FIDELITY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    # Collect source files if not provided
    if(NOT FIDELITY_SOURCES)
        file(GLOB_RECURSE FIDELITY_SOURCES
            "${FIDELITY_SOURCE_DIR}/*.cpp"
            "${FIDELITY_SOURCE_DIR}/*.cxx"
            "${FIDELITY_SOURCE_DIR}/*.cc"
            "${FIDELITY_SOURCE_DIR}/*.c"
        )
    endif()

    # Collect header files if not provided
    if(NOT FIDELITY_HEADERS)
        file(GLOB_RECURSE FIDELITY_HEADERS
            "${FIDELITY_SOURCE_DIR}/*.h"
            "${FIDELITY_SOURCE_DIR}/*.hpp"
            "${FIDELITY_SOURCE_DIR}/*.hxx"
        )
    endif()

    # Ensure we have source files
    if(NOT FIDELITY_SOURCES)
        message(FATAL_ERROR "No source files found for target ${target_name}")
    endif()

    # Create executable
    add_executable(${target_name} ${FIDELITY_SOURCES} ${FIDELITY_HEADERS})

    # Set common properties
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # Include engine headers
    target_include_directories(${target_name} PRIVATE
        "${CMAKE_SOURCE_DIR}/Source/Engine"
    )

    # Link common dependencies
    target_link_libraries(${target_name} PRIVATE
        engine
        OpenGL::GL
        ${FIDELITY_DEPENDENCIES}
    )    # Platform-specific settings
    if(WIN32)
        # These are console applications with main(), not Windows apps with WinMain
        set_target_properties(${target_name} PROPERTIES
            WIN32_EXECUTABLE FALSE
        )
        target_compile_definitions(${target_name} PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
        )
    endif()

    # Copy resources if requested
    if(FIDELITY_COPY_RESOURCES)
        fidelity_copy_resources(${target_name})
    endif()

    # IDE organization
    source_group(TREE "${FIDELITY_SOURCE_DIR}" FILES ${FIDELITY_SOURCES} ${FIDELITY_HEADERS})

    message(STATUS "Configured executable: ${target_name}")
endfunction()

# Function to copy game resources to build directory
function(fidelity_copy_resources target_name)
    # Create a global resource copying target if it doesn't exist
    if(NOT TARGET copy_fidelity_resources)
        set(RESOURCE_DIRS
            "Shaders"
            "Fonts" 
            "Models"
            "Textures"
        )

        set(copy_commands)
        foreach(resource_dir ${RESOURCE_DIRS})
            set(src_path "${CMAKE_SOURCE_DIR}/Resources/${resource_dir}")
            set(dst_path "${CMAKE_BINARY_DIR}/bin/${resource_dir}")
            
            if(EXISTS "${src_path}")
                list(APPEND copy_commands
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_path}"
                    COMMAND ${CMAKE_COMMAND} -E copy_directory "${src_path}" "${dst_path}"
                )
            endif()
        endforeach()

        # Create single custom target for all resource copying
        add_custom_target(copy_fidelity_resources
            ${copy_commands}
            COMMENT "Copying Fidelity resources to build directory"
            VERBATIM
        )
    endif()
    
    # Make the target depend on the resource copying
    add_dependencies(${target_name} copy_fidelity_resources)
endfunction()

# Function to add compiler warnings
function(fidelity_add_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE
            /W4
            /permissive-
        )
        if(FIDELITY_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE /WX)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            -Wall -Wextra -Wpedantic
        )
        if(FIDELITY_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# Function to set common debug/release configurations
function(fidelity_set_build_config target_name)
    target_compile_definitions(${target_name} PRIVATE
        $<$<CONFIG:Debug>:DEBUG _DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )

    if(MSVC)
        target_compile_options(${target_name} PRIVATE
            $<$<CONFIG:Release>:/O2 /Ob2>
            $<$<CONFIG:Debug>:/Od /Zi>
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Debug>:-g -O0>
        )
    endif()
endfunction()
