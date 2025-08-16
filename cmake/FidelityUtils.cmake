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

    # Include engine and GLFW headers
    target_include_directories(${target_name} PRIVATE
        "${CMAKE_SOURCE_DIR}/Source/Engine"
        "${CMAKE_SOURCE_DIR}/Externals/glfw/include"
    )

    # Link common dependencies
    target_link_libraries(${target_name} PRIVATE
        engine
        OpenGL::GL
        glfw
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
    # On Windows, prepare a helper script that wraps robocopy and normalizes exit codes
    if(WIN32)
        set(_FID_ROBO_SCRIPT_DIR "${CMAKE_BINARY_DIR}/cmake")
        set(_FID_ROBO_SCRIPT "${_FID_ROBO_SCRIPT_DIR}/robocopy_sync.cmake")
        file(MAKE_DIRECTORY "${_FID_ROBO_SCRIPT_DIR}")
        # Write once; safe to overwrite
        file(WRITE "${_FID_ROBO_SCRIPT}" "# Auto-generated helper to run robocopy with normalized exit codes\n"
            "if(NOT DEFINED ROBOSRC OR NOT DEFINED ROBODST)\n"
            "  message(FATAL_ERROR \"ROBOSRC and ROBODST must be provided\")\n"
            "endif()\n"
            "set(_threads \"$ENV{NUMBER_OF_PROCESSORS}\")\n"
            "if(NOT _threads)\n"
            "  set(_threads 8)\n"
            "endif()\n"
            "file(TO_NATIVE_PATH \"\${ROBOSRC}\" _SRC)\n"
            "file(TO_NATIVE_PATH \"\${ROBODST}\" _DST)\n"
            "execute_process(\n"
            "  COMMAND cmd /c robocopy \"\${_SRC}\" \"\${_DST}\" /E /COPY:DAT /DCOPY:DAT /FFT /R:2 /W:3 /NP /NFL /NDL /NJH /NJS /MT:\${_threads}\n"
            "  RESULT_VARIABLE _rc\n"
            "  OUTPUT_VARIABLE _out\n"
            "  ERROR_VARIABLE _err\n"
            ")\n"
            "if(_rc LESS 8)\n"
            "  message(STATUS \"Robocopy OK (\${_rc}) -> \${ROBODST}\")\n"
            "else()\n"
            "  message(FATAL_ERROR \"Robocopy failed (\${_rc}) -> \${ROBODST}\\nSTDOUT: \n\${_out}\\nSTDERR: \n\${_err}\")\n"
            "endif()\n"
        )
    endif()

    # Create a global resource copying target if it doesn't exist
    if(NOT TARGET copy_fidelity_resources)
        set(RESOURCE_DIRS
            "Shaders"
            "Fonts" 
            "Models"
            "Textures"
        )

        set(copy_commands)

        list(LENGTH RESOURCE_DIRS _fid_total_dirs)
        set(_fid_dir_index 0)
        foreach(resource_dir ${RESOURCE_DIRS})
            math(EXPR _fid_dir_index "${_fid_dir_index} + 1")
            set(src_path "${CMAKE_SOURCE_DIR}/Resources/${resource_dir}")
            # Copy to bin/, bin/Release and bin/Debug to handle different output directories
            set(dst_path_root "${CMAKE_BINARY_DIR}/bin/${resource_dir}")
            set(dst_path_release "${CMAKE_BINARY_DIR}/bin/Release/${resource_dir}")
            set(dst_path_debug "${CMAKE_BINARY_DIR}/bin/Debug/${resource_dir}")

            if(EXISTS "${src_path}")
                # Count files for a slightly more informative log
                file(GLOB_RECURSE _fid_files_count
                    LIST_DIRECTORIES false
                    "${src_path}/*"
                )
                list(LENGTH _fid_files_count _fid_nfiles)

                # Start message for this resource directory
                list(APPEND copy_commands
                    COMMAND ${CMAKE_COMMAND} -E echo "-- [${_fid_dir_index}/${_fid_total_dirs}] Syncing '${resource_dir}' (${_fid_nfiles} files)"
                )

                # Root
                list(APPEND copy_commands
                    COMMAND ${CMAKE_COMMAND} -E echo "   -> bin/${resource_dir}"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_path_root}"
                )
                if(WIN32)
                    # Use robocopy via helper script for incremental sync on Windows
                    list(APPEND copy_commands
                        COMMAND ${CMAKE_COMMAND} -DROBOSRC="${src_path}" -DROBODST="${dst_path_root}" -P "${_FID_ROBO_SCRIPT}"
                    )
                else()
                    # Fallback: copy entire directory (may overwrite unchanged files)
                    list(APPEND copy_commands
                        COMMAND ${CMAKE_COMMAND} -E copy_directory "${src_path}" "${dst_path_root}"
                    )
                endif()

                # Release
                list(APPEND copy_commands
                    COMMAND ${CMAKE_COMMAND} -E echo "   -> bin/Release/${resource_dir}"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_path_release}"
                )
                if(WIN32)
                    list(APPEND copy_commands
                        COMMAND ${CMAKE_COMMAND} -DROBOSRC="${src_path}" -DROBODST="${dst_path_release}" -P "${_FID_ROBO_SCRIPT}"
                    )
                else()
                    list(APPEND copy_commands
                        COMMAND ${CMAKE_COMMAND} -E copy_directory "${src_path}" "${dst_path_release}"
                    )
                endif()

                # Debug
                list(APPEND copy_commands
                    COMMAND ${CMAKE_COMMAND} -E echo "   -> bin/Debug/${resource_dir}"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_path_debug}"
                )
                if(WIN32)
                    list(APPEND copy_commands
                        COMMAND ${CMAKE_COMMAND} -DROBOSRC="${src_path}" -DROBODST="${dst_path_debug}" -P "${_FID_ROBO_SCRIPT}"
                    )
                else()
                    list(APPEND copy_commands
                        COMMAND ${CMAKE_COMMAND} -E copy_directory "${src_path}" "${dst_path_debug}"
                    )
                endif()

                # Done message for this resource directory
                list(APPEND copy_commands
                    COMMAND ${CMAKE_COMMAND} -E echo "-- Done '${resource_dir}'"
                )
            else()
                list(APPEND copy_commands
                    COMMAND ${CMAKE_COMMAND} -E echo "-- [${_fid_dir_index}/${_fid_total_dirs}] Skipping '${resource_dir}' (not found)"
                )
            endif()
        endforeach()

        # Create single custom target for all resource copying
        add_custom_target(copy_fidelity_resources
            ${copy_commands}
            COMMENT "Copying Fidelity resources to build directories"
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
