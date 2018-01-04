file(GLOB TestsSrc 
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	)

source_group(Tests FILES ${TestsSrc})

add_executable(Tests ${TestsSrc})

target_link_libraries(Tests ${SDL2_LIBRARIES})
target_link_libraries(Tests Engine)