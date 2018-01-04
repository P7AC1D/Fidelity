cmake_minimum_required(VERSION 2.8)

project(Fidelity)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR})

find_package(SDL2 REQUIRED)

IF (WIN32)
set(GLEW_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/Externals/glew-2.1.0/include")
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(GLEW_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/Externals/glew-2.1.0/lib/Release/x64/glew32.lib")
else ()
  set(GLEW_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/Externals/glew-2.1.0/lib/Release/Win32/glew32.lib")
endif ()
find_package(GLEW REQUIRED)
ENDIF()

add_subdirectory(Source)