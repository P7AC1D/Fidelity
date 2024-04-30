CMAKE_MINIMUM_REQUIRED( VERSION 3.10 )

project(Fidelity)

set(ENTITYX_BUILD_SHARED FALSE)
set(CMAKE_CXX_STANDARD 17)

set(ASSIMP_WARNINGS_AS_ERRORS OFF)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory ("Source/Engine")
add_subdirectory ("Source/Test3D")
add_subdirectory ("Source/Sponza")
add_subdirectory ("Source/CullingTest")
add_subdirectory ("Source/Tests")

add_subdirectory ("Externals/glfw")
add_subdirectory ("Externals/assimp")
add_subdirectory ("Externals/glad")
add_subdirectory ("Externals/glm")