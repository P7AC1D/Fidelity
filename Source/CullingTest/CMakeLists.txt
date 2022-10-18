cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

include_directories ("${CMAKE_SOURCE_DIR}/Source/Engine/")

file(GLOB CullingTestSrc ${CMAKE_CURRENT_SOURCE_DIR}/**/*.*, ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

source_group(CullingTest FILES ${CullingTestSrc})
add_executable(CullingTest ${CullingTestSrc})

target_link_libraries (CullingTest engine)

set_target_properties(CullingTest PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
set_target_properties(CullingTest PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(CullingTest ${OPENGL_LIBRARIES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Shaders/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Fonts/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Fonts/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Models/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Models/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../Resources/Textures/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Textures/)