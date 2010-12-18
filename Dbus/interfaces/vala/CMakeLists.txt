
########### install files ###############

if ("${VALA_FOUND}" STREQUAL "TRUE")
	# first, precompile the Vala code into C code.
	set (CDAPPLET "CDApplet")
	execute_process(COMMAND
		${VALA_EXECUTABLE} -q -C --disable-warnings --disable-dbus-transformation
		--pkg gio-2.0
		--internal-vapi=CDApplet.vapi
		--internal-header=CDApplet.h
		-o CDApplet.c
		${CMAKE_CURRENT_BINARY_DIR}/CDApplet.vala)
	
	# valac is a bad boy, it messes up the signal names.
	execute_process(COMMAND
		sed -i "s/OnClick/on_click/g; s/OnMiddleClick/on_middle_click/g; s/OnScroll/on_scroll/g; s/OnBuildMenu/on_build_menu/g; s/OnMenuSelect/on_menu_select/g; s/OnDropData/on_drop_data/g; s/OnAnswer/on_answer/g; s/OnAnswerDialog/on_answer_dialog/g; s/OnShortkey/on_shortkey/g; s/OnChangeFocus/on_change_focus/g; s/OnReloadModule/on_reload_module/g; s/OnStopModule/on_stop_module/g" ${CMAKE_CURRENT_BINARY_DIR}/CDApplet.c)
	
	# now, compile it as any normal C lib.
	add_library (${CDAPPLET} SHARED CDApplet.c)
	add_definitions (-fPIC)
	
	pkg_check_modules ("CDAPPLET_DEPS" "gobject-2.0" "gio-2.0")
	
	include_directories (
		${CDAPPLET_DEPS_INCLUDE_DIRS})
	
	link_directories (
		${CDAPPLET_DEPS_LIBRARY_DIRS})
	
	target_link_libraries (${CDAPPLET}
		${CDAPPLET_DEPS_LIBRARIES})
	
	# .so goes with other libs (to be found on runtime)
	install (TARGETS ${CDAPPLET} DESTINATION "${libdir}")
	# .h goes in the dbus data dir
	install(FILES CDApplet.h DESTINATION "${dbusdatadir}")
	# .pc goes with others
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/CDApplet.pc.in ${CMAKE_CURRENT_BINARY_DIR}/CDApplet.pc)
	install (FILES  ${CMAKE_CURRENT_BINARY_DIR}/CDApplet.pc DESTINATION ${libdir}/pkgconfig)
	# and .vapi with other vapi files.
	GET_FILENAME_COMPONENT(VALA_EXECUTABLE_PATH ${VALA_EXECUTABLE} PATH) # -> /usr/bin
	GET_FILENAME_COMPONENT(VALA_SHARE_PATH ${VALA_EXECUTABLE_PATH}/../share ABSOLUTE)  # -> /usr/share
	STRING (REGEX REPLACE "([^ ]+)\\.[0-9]*" "\\1" VALA_VERSION_SHORT "${VALA_VERSION}")  # remove micro: 0.10.0 -> 0.10
	GET_FILENAME_COMPONENT(VALA_VAPI_PATH ${VALA_SHARE_PATH}/vala-${VALA_VERSION_SHORT}/vapi ABSOLUTE)
	if (NOT EXISTS ${VALA_VAPI_PATH})
		GET_FILENAME_COMPONENT(VALA_VAPI_PATH ${VALA_SHARE_PATH}/vala/vapi ABSOLUTE)
	endif()
	install (FILES ${CDAPPLET}.vapi DESTINATION ${VALA_VAPI_PATH})
	install (FILES ${CDAPPLET}.deps DESTINATION ${VALA_VAPI_PATH})
endif()
