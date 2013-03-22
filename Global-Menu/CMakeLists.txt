add_subdirectory(src)
add_subdirectory(data)
 
if (PYTHON2_FOUND OR PYTHON3_FOUND)
	add_subdirectory(registrar)
endif()
