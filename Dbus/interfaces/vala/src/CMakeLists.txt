# if we have produced new .c, .h, .vapi, .deps in build dir:
if ("${VALAC_FOUND}" STREQUAL "TRUE" AND NOT "${WITH_VALA_SRC}" STREQUAL "yes")
	set (VALA_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR})
else()
	set (VALA_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_compile_options (-fPIC -Wno-all -Wno-incompatible-pointer-types)

# now, compile it as any normal C lib.
add_library (${CDAPPLET} SHARED ${VALA_SRC_DIR}/${CDAPPLET}.c)
STRING (REGEX REPLACE "\\..*" "" SOVERSION_VALA "${VERSION_VALA}")
set_target_properties (${CDAPPLET} PROPERTIES
	# create *nix style library versions + symbolic links
	VERSION ${VERSION_VALA}
	SOVERSION ${SOVERSION_VALA}
	# allow creating static and shared libs without conflicts
	#CLEAN_DIRECT_OUTPUT 1
	# avoid conflicts between library and binary target names
	#OUTPUT_NAME ${PROJECT_NAME}
)
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
install (FILES ${VALA_SRC_DIR}/${CDAPPLET}.h DESTINATION "${dbusdatadir}")

# Use the VAPI path from ValaVapiPath.txt file:
find_program (HEAD_EXECUTABLE head)
execute_process(COMMAND ${HEAD_EXECUTABLE} -n 1 ${VALA_SRC_DIR}/ValaVapiPath.txt
	OUTPUT_VARIABLE VALA_VAPI_PATH)
STRING (REGEX REPLACE "\n" "" VALA_VAPI_PATH ${VALA_VAPI_PATH})

get_filename_component(VALA_VAPI_PATH "${VALA_SHARE_PATH}/${VALA_VAPI_PATH}" ABSOLUTE)
install (FILES "${VALA_SRC_DIR}/${CDAPPLET}.vapi" DESTINATION "${VALA_VAPI_PATH}")
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../${CDAPPLET}.deps" DESTINATION "${VALA_VAPI_PATH}")
