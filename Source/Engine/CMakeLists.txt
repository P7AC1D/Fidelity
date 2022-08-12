cmake_minimum_required (VERSION 3.8 FATAL_ERROR)
PROJECT (engine)

SET(CMAKE_GENERATOR_PLATFORM x64)

file (GLOB_RECURSE EngineSrc ${CMAKE_CURRENT_SOURCE_DIR}/**/*.*)
add_library (engine STATIC ${EngineSrc})
set_target_properties(engine PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
set_target_properties(engine PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(engine PUBLIC "${CMAKE_SOURCE_DIR}/Externals/glfw/include")
target_include_directories(engine PUBLIC "${CMAKE_SOURCE_DIR}/Externals/assimp/include/")
target_include_directories(engine PUBLIC "${CMAKE_SOURCE_DIR}/Externals/glad/")
target_include_directories(engine PUBLIC "${Vulkan_INCLUDE_DIR}")

target_link_libraries (engine glfw)
target_link_libraries (engine assimp)
target_link_libraries (engine glad)
target_link_libraries (engine ${Vulkan_LIBRARIES})