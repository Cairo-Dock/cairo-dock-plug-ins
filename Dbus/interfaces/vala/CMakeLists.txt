
########### install files ###############

# ${CMAKE_INSTALL_PREFIX} can change... so if we want to install new vala files,
# we have to use this variable => we can put this variable into ValaVapiPath.txt.in...
set (VALA_SHARE_PATH "${CMAKE_INSTALL_PREFIX}/share")
set (CDAPPLET "CDApplet")

# if we want to produce new .c, .h, .vapi, .deps:
if ("${VALAC_FOUND}" STREQUAL "TRUE")
	# first, precompile the Vala code into C code.
	execute_process(COMMAND
		${VALA_EXECUTABLE} -q -C --disable-warnings --disable-dbus-transformation
		--pkg gio-2.0
		--vapi=${CMAKE_CURRENT_SOURCE_DIR}/src/${CDAPPLET}.vapi
		--header=${CMAKE_CURRENT_SOURCE_DIR}/src/${CDAPPLET}.h
		-o ${CDAPPLET}.c
		${CMAKE_CURRENT_SOURCE_DIR}/${CDAPPLET}.vala)
	# it seems that valac can only produce the output into the current directory.
	execute_process(COMMAND mv ${CDAPPLET}.c ${CMAKE_CURRENT_SOURCE_DIR}/src/${CDAPPLET}.c)
	
	# valac is a bad boy, it messes up the signal names.
	execute_process(COMMAND
		sed -i "s/OnClick/on_click/g; s/OnMiddleClick/on_middle_click/g; s/OnScroll/on_scroll/g; s/OnBuildMenu/on_build_menu/g; s/OnMenuSelect/on_menu_select/g; s/OnDropData/on_drop_data/g; s/OnAnswerDialog/on_answer_dialog/g; s/OnAnswer/on_answer/g; s/OnShortkey/on_shortkey/g; s/OnChangeFocus/on_change_focus/g; s/OnReloadModule/on_reload_module/g; s/OnStopModule/on_stop_module/g" ${CMAKE_CURRENT_SOURCE_DIR}/src/CDApplet.c)

	# .vapi install dir:
	# GET_FILENAME_COMPONENT(VALA_EXECUTABLE_PATH ${VALA_EXECUTABLE} PATH) # -> /usr/bin
	# GET_FILENAME_COMPONENT(VALA_SHARE_PATH ${VALA_EXECUTABLE_PATH}/../share ABSOLUTE)  # -> /usr/share
	# with my valac-0.12, the version is Vala 0.11.3 because it's still under a development release => gnome versions.
	string (REGEX REPLACE "\\..*" "" VALA_VERSION_MAJOR "${VALA_VERSION}")
	string (REGEX REPLACE "([^ ]+)\\.[0-9]*" "\\1" VALA_VERSION_MINOR "${VALA_VERSION}")  # remove nano: 0.10.0 -> 0.10
	string (REGEX REPLACE ".*\\." "" VALA_VERSION_MINOR "${VALA_VERSION_MINOR}")
	string (REGEX REPLACE "-.*" "" VALA_VERSION_MINOR "${VALA_VERSION_MINOR}")
	math (EXPR VALA_VERSION_MINOR_MODULO "${VALA_VERSION_MINOR} % 2")
	if ("${VALA_VERSION_MINOR_MODULO}" STREQUAL "1")
		math (EXPR VALA_VERSION_MINOR "${VALA_VERSION_MINOR} + 1")
	endif()
	get_filename_component(VALA_VAPI_PATH "${VALA_SHARE_PATH}/vala-${VALA_VERSION_MAJOR}.${VALA_VERSION_MINOR}/vapi" ABSOLUTE)
	if (NOT EXISTS ${VALA_VAPI_PATH})
		set(VALA_VAPI_PATH "vala/vapi")
	else()
		set(VALA_VAPI_PATH "vala-${VALA_VERSION_MAJOR}.${VALA_VERSION_MINOR}/vapi")
	endif()
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ValaVapiPath.txt.in ${CMAKE_CURRENT_SOURCE_DIR}/src/ValaVapiPath.txt)
endif()

# now we can install vala files.
add_subdirectory(src)
# .pc goes with others
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/${CDAPPLET}.pc.in ${CMAKE_CURRENT_BINARY_DIR}/${CDAPPLET}.pc)
install (FILES  ${CMAKE_CURRENT_BINARY_DIR}/${CDAPPLET}.pc DESTINATION ${libdir}/pkgconfig)
