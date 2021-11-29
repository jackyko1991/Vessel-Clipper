list(APPEND UTILS_LIBRARIES 
	Branch
	# Measurements
	)

foreach(ONE ${UTILS_LIBRARIES})
	add_subdirectory(${ONE})
	include_directories(${${ONE}_SOURCE_DIR})
	include_directories(${${ONE}_BINARY_DIR})
endforeach()