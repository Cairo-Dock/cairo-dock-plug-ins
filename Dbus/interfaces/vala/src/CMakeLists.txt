# now, compile it as any normal C lib.
add_library (${CDAPPLET} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/${CDAPPLET}.c)
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
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${CDAPPLET}.h DESTINATION "${dbusdatadir}")

# Use the VAPI path from ValaVapiPath.txt file:
find_program (HEAD_EXECUTABLE head)
execute_process(COMMAND ${HEAD_EXECUTABLE} -n 1 ${CMAKE_CURRENT_SOURCE_DIR}/ValaVapiPath.txt
	OUTPUT_VARIABLE VALA_VAPI_PATH)
STRING (REGEX REPLACE "\n" "" VALA_VAPI_PATH ${VALA_VAPI_PATH})

get_filename_component(VALA_VAPI_PATH "${VALA_SHARE_PATH}/${VALA_VAPI_PATH}" ABSOLUTE)
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/${CDAPPLET}.vapi" DESTINATION "${VALA_VAPI_PATH}")
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../${CDAPPLET}.deps" DESTINATION "${VALA_VAPI_PATH}")