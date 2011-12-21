configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CDApplet.py.in ${CMAKE_CURRENT_BINARY_DIR}/CDApplet.py)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CairoDock.py.in ${CMAKE_CURRENT_BINARY_DIR}/CairoDock.py)

if (NOT "${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
	# copy *.py to current binary dir (cleaner)
	execute_process(COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/setup.py" ${CMAKE_CURRENT_BINARY_DIR})
	exec_program("${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}" ARGS "setup.py" "-q" "build")
endif()

########### install files ###############
# Run a custom script with 'make install' 
add_custom_target(python_pyc ALL)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PythonInstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/PythonInstall.cmake)
set_target_properties(python_pyc PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/PythonInstall.cmake)
# execute_process(COMMAND python setup.py install ${DEBIAN_INSTALL_LAYOUT})
