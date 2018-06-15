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

file(GLOB ImageSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Image/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Image/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Image/*.cpp
	)
	
file(GLOB MathsSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Maths/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Maths/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Maths/*.cpp
	)
	
file(GLOB OverlaySrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Overlay/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Overlay/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Overlay/*.cpp
	)

file(GLOB RenderingSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.cpp
	)
	
file(GLOB RenderApiSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/RenderApi/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/RenderApi/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/RenderApi/*.cpp
	)
	
file(GLOB RenderApiGLSrc
	${CMAKE_CURRENT_SOURCE_DIR}/RenderApi/GL/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/RenderApi/GL/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/RenderApi/GL/*.cpp
	)
	
file(GLOB GeometrySrc
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Geometry/*.cpp
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
source_group(Images FILES ${ImageSrc})
source_group(Maths FILES ${MathsSrc})
source_group(Rendering FILES ${RenderingSrc}) 
source_group(RenderApi FILES ${RenderApiSrc})
source_group(RenderApi\\GL FILES ${RenderApiGLSrc})
source_group(Geometry FILES ${GeometrySrc})
source_group(SceneManagement FILES ${SceneManagementSrc})
source_group(Overlay FILES ${OverlaySrc})
source_group(Utility FILES ${UtilitySrc})
source_group(Shaders FILES ${ShaderSrc})
source_group(Input FILES ${InputSrc})

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${ASSIMP_INCLUDE_DIR})
include_directories(${IMGUI_LIBRARY_DIR})

IF (APPLE)
include_directories(${OPENGL_INCLUDE_DIRS})
ENDIF()

add_library(Engine STATIC ${ComponentsSrc} ${CoreSrc} ${GeometrySrc} ${ImageSrc} ${MathsSrc} ${OverlaySrc} ${RenderApiSrc} ${RenderApiGLSrc} ${RenderingSrc} ${SceneManagementSrc} ${UtilitySrc} ${ShaderSrc} ${InputSrc})