if (with_python3)
	if (NOT "${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
		# copy *.py to current binary dir (cleaner)
		execute_process(COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/CDBashApplet.py" ${CMAKE_CURRENT_BINARY_DIR})
	endif()
	#  byte-compile the python scripts to .pyc and install them
	execute_process(COMMAND "${PYTHON3_EXECUTABLE}" "-m" "compileall" "CDBashApplet.py" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
	install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/__pycache__" DESTINATION ${dbusdatadir})
	# install the original python script
	install(FILES "CDBashApplet.py" DESTINATION ${dbusdatadir})
	# and the bash functions goes into the data dir of the applet (didn't find a better place).
	install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/CDBashApplet.sh
		DESTINATION ${dbusdatadir}
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE)
endif()

