file(GLOB Test3DSrc 
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  )

source_group(Test3D FILES ${Test3DSrc})

add_executable(Test3D ${Test3DSrc})

target_link_libraries(Test3D ${SDL2_LIBRARIES})

IF (WIN32)
target_link_libraries(Test3D ${GLEW_LIBRARY})
ENDIF()

target_link_libraries(Test3D Engine)