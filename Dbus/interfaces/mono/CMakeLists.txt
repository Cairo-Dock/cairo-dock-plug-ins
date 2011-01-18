	
########### install files ###############

if ("${MONO_FOUND}" STREQUAL "TRUE")
	execute_process(COMMAND ${GMCS_EXECUTABLE}
		-target:library
		-pkg:glib-sharp-2.0 -pkg:ndesk-dbus-1.0 -pkg:ndesk-dbus-glib-1.0
		-out:${CMAKE_CURRENT_BINARY_DIR}/CDApplet.dll
		${CMAKE_CURRENT_SOURCE_DIR}/CDApplet.cs ${CMAKE_CURRENT_SOURCE_DIR}/ISubApplet.cs ${CMAKE_CURRENT_SOURCE_DIR}/IApplet.cs)
	### find how to register to GAC ...
	set (MONO_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/cli")
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/CDApplet.dll
		DESTINATION ${MONO_LIB_DIR})
endif()

