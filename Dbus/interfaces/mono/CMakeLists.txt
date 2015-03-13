	
########### install files ###############
if (GLIB_SHARP_3)
	execute_process(COMMAND ${GMCS_EXECUTABLE}
		-target:library
		-pkg:dbus-sharp-2.0 -pkg:dbus-sharp-glib-2.0 -pkg:glib-sharp-3.0
		-out:${CMAKE_CURRENT_BINARY_DIR}/CDApplet.dll
		${CMAKE_CURRENT_SOURCE_DIR}/CDApplet.cs ${CMAKE_CURRENT_SOURCE_DIR}/ISubApplet.cs ${CMAKE_CURRENT_SOURCE_DIR}/IApplet.cs)
else()
	execute_process(COMMAND ${GMCS_EXECUTABLE}
		-target:library
		-pkg:dbus-sharp-2.0 -pkg:dbus-sharp-glib-2.0 -pkg:glib-sharp-2.0
		-out:${CMAKE_CURRENT_BINARY_DIR}/CDApplet.dll
		${CMAKE_CURRENT_SOURCE_DIR}/CDApplet.cs ${CMAKE_CURRENT_SOURCE_DIR}/ISubApplet.cs ${CMAKE_CURRENT_SOURCE_DIR}/IApplet.cs)
endif()
### find how to register to GAC ...
set (MONO_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/cli/cairo-dock-plug-ins")
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/CDApplet.dll
	DESTINATION ${MONO_LIB_DIR})

