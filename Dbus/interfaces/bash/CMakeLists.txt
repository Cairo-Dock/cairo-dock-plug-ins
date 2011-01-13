
########### install files ###############

if ("${PYTHON_FOUND}" STREQUAL "TRUE")
	# the python script goes with other python scripts.
	if (NOT "${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
		# copy *.py to current binary dir (cleaner)
		execute_process(COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/setup.py" "${CMAKE_CURRENT_SOURCE_DIR}/CDBashApplet.py" ${CMAKE_CURRENT_BINARY_DIR})
		exec_program("${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}" ARGS "setup.py" "-q" "build")
	endif()
	# Run a custom script with 'make install' 
	add_custom_target(bash_pyc ALL)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BashInstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/BashInstall.cmake)
	set_target_properties(bash_pyc PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/BashInstall.cmake)
	
	# and the bash functions goes into the data dir of the applet (didn't find a better place).
	install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/CDBashApplet.sh
		DESTINATION ${dbusdatadir}
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE)
endif()
