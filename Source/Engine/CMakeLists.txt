file(GLOB ComponentsSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Components/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Components/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Components/*.cpp
	)
	
file(GLOB CoreSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Core/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp
	)
	
file(GLOB MathsSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Maths/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Maths/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Maths/*.cpp
	)

file(GLOB RenderingSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.cpp
	)

file(GLOB SceneManagementSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/SceneManagement/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/SceneManagement/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/SceneManagement/*.cpp
	)
	
file(GLOB InputSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Input/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Input/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Input/*.cpp
	)

file(GLOB UiSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/UI/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/UI/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/UI/*.cpp
	)

file(GLOB UtilitySrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.cpp
	)

file(GLOB ShaderSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.shader
	)

source_group(Components FILES ${ComponentsSrc})
source_group(Core FILES ${CoreSrc})
source_group(Maths FILES ${MathsSrc})
source_group(Rendering FILES ${RenderingSrc})
source_group(SceneManagement FILES ${SceneManagementSrc})
source_group(UI FILES ${UiSrc})
source_group(Utility FILES ${UtilitySrc})
source_group(Shaders FILES ${ShaderSrc})
source_group(Input FILES ${InputSrc})

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIRS})

IF (APPLE)
include_directories(${OPENGL_INCLUDE_DIRS})
ENDIF()

add_library(Engine STATIC ${ComponentsSrc} ${CoreSrc} ${MathsSrc} ${RenderingSrc} ${SceneManagementSrc} ${UiSrc} ${UtilitySrc} ${ShaderSrc} ${InputSrc})