
########### install files ###############

if ("${PYTHONLIBS_FOUND}" STREQUAL "TRUE")
	# the python script goes with other python scripts.
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/CDBashApplet.py
		DESTINATION ${PYTHON_SITE_PACKAGE})
	
	# and the bash functions gors into the data dir of the applet (didn't find a better place).
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/CDBashApplet.sh
		DESTINATION ${dbusdatadir}
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE)
endif()
