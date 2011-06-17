
########### install files ###############

#if (NOT "${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
	# copy *.py to current binary dir (cleaner)
#	execute_process(COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/setup.py" "${CMAKE_CURRENT_SOURCE_DIR}/status-notifier-watcher.py" ${CMAKE_CURRENT_BINARY_DIR})
#	exec_program("${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}" ARGS "setup.py" "-q" "build")
#endif()
# Run a custom script with 'make install' 
#add_custom_target(watcher_pyc ALL)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PythonInstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/PythonInstall.cmake)
#set_target_properties(watcher_pyc PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/PythonInstall.cmake)
# execute_process(COMMAND python setup.py install ${DEBIAN_INSTALL_LAYOUT})

install(FILES status-notifier-watcher.py
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	RENAME status-notifier-watcher
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ)
