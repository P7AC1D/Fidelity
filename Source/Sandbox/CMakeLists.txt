cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

include_directories ("${CMAKE_SOURCE_DIR}/Source/Engine/")

file(GLOB_RECURSE Src ${CMAKE_CURRENT_SOURCE_DIR}/**/*.*, ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

source_group(Sandbox FILES ${Src})
add_executable(Sandbox ${Src})

target_link_libraries (Sandbox engine)

set_target_properties(Sandbox PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
set_target_properties(Sandbox PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(Sandbox ${OPENGL_LIBRARIES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Shaders/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Fonts/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Fonts/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Models/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Models/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Textures/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Textures/)