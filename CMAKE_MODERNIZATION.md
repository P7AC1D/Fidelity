# CMake Modernization Guide

## Overview

This document outlines the modernization of the Fidelity Engine CMake build system to address compatibility issues and follow modern CMake best practices.

## Issues Addressed

### 1. CMake Version Compatibility
- **Problem**: Multiple CMakeLists.txt files used outdated minimum versions (3.2, 3.8, 3.10)
- **Solution**: Updated to CMake 3.21 for modern features and consistent behavior
- **Impact**: Eliminates deprecation warnings and enables modern CMake features

### 2. External Dependencies Compatibility
- **Problem**: GLM submodule requires CMake < 3.5, causing build failures
- **Solution**: Used `CMAKE_POLICY_VERSION_MINIMUM` and policy isolation with `cmake_policy(PUSH/POP)`
- **Impact**: External submodules can use their required CMake versions without conflicts

### 3. Policy Warnings
- **Problem**: Modern CMake policies not being set, causing warnings
- **Solution**: Explicitly set modern policies (CMP0077, CMP0175) in main CMakeLists.txt
- **Impact**: Eliminates policy warnings and ensures consistent behavior

### 4. Error Handling
- **Problem**: Poor error handling for missing dependencies and build failures
- **Solution**: Added comprehensive error checking with descriptive messages
- **Impact**: Better developer experience with clear error messages

## Key Modernization Changes

### CMake Structure
```
CMakeLists.txt              # Main project configuration
cmake/
├── FidelityUtils.cmake     # Shared utilities and functions
Source/
├── Engine/CMakeLists.txt   # Engine library
├── Test3D/CMakeLists.txt   # Test applications
├── Sponza/CMakeLists.txt   # Sponza demo
└── CullingTest/CMakeLists.txt # Culling test
```

### Modern Features Used
1. **Target-based approach**: Using `target_*` commands instead of global settings
2. **Generator expressions**: For configuration-specific settings
3. **Interface libraries**: For header-only dependencies
4. **Proper scoping**: Avoiding global variables where possible
5. **Error handling**: Comprehensive validation and error messages

### External Dependencies Strategy
- **Don't modify external submodules**: Use policy isolation instead
- **Version compatibility**: Handle different CMake requirements gracefully
- **EXCLUDE_FROM_ALL**: Prevent external targets from appearing in main build
- **Policy isolation**: Use `cmake_policy(PUSH/POP)` to isolate external requirements

## Developer Guidelines

### Adding New External Dependencies
1. Check if the dependency has CMake support
2. Add to `Externals/` directory as submodule
3. Use policy isolation in main CMakeLists.txt:
   ```cmake
   cmake_policy(PUSH)
   add_subdirectory(Externals/new_lib EXCLUDE_FROM_ALL)
   cmake_policy(POP)
   ```

### Creating New Components
1. Use the utilities from `cmake/FidelityUtils.cmake`
2. Follow the target-based approach
3. Set appropriate C++ standard and warnings
4. Add proper error handling

### Best Practices
- Use `find_package()` for system dependencies
- Use `target_link_libraries()` with visibility specifiers
- Avoid `file(GLOB)` for source files in production code
- Always validate dependencies before using them
- Use meaningful error messages

## Build Configuration

### Supported Configurations
- **Debug**: Full debug info, no optimizations
- **Release**: Full optimizations, no debug info
- **RelWithDebInfo**: Optimizations with debug info
- **MinSizeRel**: Size optimizations

### Options
- `FIDELITY_BUILD_TESTS`: Build test executables (default: ON)
- `FIDELITY_ENABLE_WARNINGS`: Enable compiler warnings (default: ON)
- `FIDELITY_WARNINGS_AS_ERRORS`: Treat warnings as errors (default: OFF)

### Platform Support
- **Windows**: Visual Studio 2019+ (MSVC, Clang)
- **Linux**: GCC 9+, Clang 10+
- **macOS**: Xcode 11+, Clang 10+

## Troubleshooting

### Common Issues

1. **GLM CMake Error**
   - **Error**: "Compatibility with CMake < 3.5 has been removed"
   - **Solution**: Policy isolation is now handled automatically

2. **Policy Warnings**
   - **Error**: Various CMP0xxx policy warnings
   - **Solution**: Modern policies are set in main CMakeLists.txt

3. **Missing Dependencies**
   - **Error**: Package not found errors
   - **Solution**: Check error messages for specific requirements

### Debug Build Issues
1. Enable verbose output: `cmake --build . --verbose`
2. Check CMake configuration: `cmake -LAH .`
3. Review error logs in build directory

## Migration Notes

### For Existing Projects
- Update your local CMakeLists.txt files to use modern practices
- Don't modify external submodules
- Test builds on all target platforms
- Update CI/CD scripts if necessary

### Backward Compatibility
- External submodules continue to work unchanged
- Existing build scripts should work with minimal changes
- Old CMake versions (3.21+) are still supported through policy settings

## Future Improvements

1. **Package Management**: Consider using vcpkg or Conan for dependencies
2. **Testing**: Add CTest integration for automated testing
3. **Documentation**: Generate API documentation with Doxygen
4. **Packaging**: Add CPack configuration for distribution
5. **Cross-compilation**: Add support for embedded targets

## References

- [Modern CMake Practices](https://cliutils.gitlab.io/modern-cmake/)
- [CMake Policies](https://cmake.org/cmake/help/latest/manual/cmake-policies.7.html)
- [Target-based CMake](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html)
