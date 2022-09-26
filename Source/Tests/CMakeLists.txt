file(GLOB TestsSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	)

source_group(Tests FILES ${TestsSrc})

add_executable(Tests ${TestsSrc})

target_link_libraries(Tests engine)
target_link_libraries (Tests glm)

target_include_directories(engine PUBLIC "${CMAKE_SOURCE_DIR}/Externals/glm/glm")