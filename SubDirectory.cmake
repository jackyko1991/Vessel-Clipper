list(APPEND PROJECT_LIBRARIES 
	MainWindow 
	Core
	CenterlinesInfoWidget
	)

foreach(ONE ${PROJECT_LIBRARIES})
	add_subdirectory(${ONE})
	include_directories(${${ONE}_SOURCE_DIR})
	include_directories(${${ONE}_BINARY_DIR})
endforeach()


add_subdirectory(Utils)
add_subdirectory(ThirdParty)