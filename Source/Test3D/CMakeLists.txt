file(GLOB Test3DSrc 
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  )

source_group(Test3D FILES ${Test3DSrc})
add_executable(Test3D ${Test3DSrc})

IF (WIN32)
  target_link_libraries(Test3D ${GLEW_LIBRARIES}/lib/Release/x64/glew32.lib)
  file(COPY ${GLEW_LIBRARIES}/bin/Release/x64/glew32.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
  file(COPY ${GLEW_LIBRARIES}/bin/Release/x64/glew32.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  file(COPY ${SDL2_INCLUDE_DIR}/../lib/x64/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
  file(COPY ${SDL2_INCLUDE_DIR}/../lib/x64/SDL2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
ENDIF()

target_link_libraries(Test3D ${SDL2_LIBRARY})
target_link_libraries(Test3D ${OPENGL_LIBRARIES})
target_link_libraries(Test3D Engine)