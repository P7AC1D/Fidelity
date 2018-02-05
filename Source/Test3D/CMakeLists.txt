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
  file(COPY ${ASSIMP_LIBRARY}/assimp-vc140-mt.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
  file(COPY ${ASSIMP_LIBRARY}/assimp-vc140-mt.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)


target_link_libraries(Test3D ${ASSIMP_LIBRARY}/assimp-vc140-mt.lib)
ENDIF()

IF (APPLE)
  target_link_libraries(Test3D ${ASSIMP_LIBRARY}/libassimp.4.1.0.dylib)
ENDIF()

if (MSVC)
  IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set_target_properties(Test3D PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug")
  ELSE()
    set_target_properties(Test3D PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release")
  ENDIF()
ENDIF()

target_link_libraries(Test3D ${SDL2_LIBRARY})
target_link_libraries(Test3D ${OPENGL_LIBRARIES})
target_link_libraries(Test3D Engine)