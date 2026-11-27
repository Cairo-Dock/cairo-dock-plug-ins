if (with_python3)
	if (NOT "${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
		# copy *.py to current binary dir (cleaner)
		execute_process(COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/CDApplet.py" ${CMAKE_CURRENT_BINARY_DIR})
	endif()

	#  byte-compile the python scripts to .pyc and install them
	execute_process(COMMAND "${PYTHON3_EXECUTABLE}" "-m" "compileall" "CDApplet.py" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
	install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/__pycache__" DESTINATION ${dbusdatadir})
	# install the original python scripts
	install(FILES "CDApplet.py" DESTINATION ${dbusdatadir})
endif()
