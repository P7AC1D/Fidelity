cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project(Fidelity)

set(ENTITYX_BUILD_SHARED FALSE)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory ("Source/Engine")
add_subdirectory ("Source/Test3D")
add_subdirectory ("Source/Tests")

add_subdirectory ("Externals/glfw")
add_subdirectory ("Externals/assimp")
add_subdirectory ("Externals/glad")