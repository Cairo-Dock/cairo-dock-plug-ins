
########### sources ###############

SET(cd-clock_LIB_SRCS
	applet-struct.h
	applet-init.c 			applet-init.h
	applet-config.c 			applet-config.h
	applet-notifications.c 		applet-notifications.h
	applet-draw.c 			applet-draw.h
	applet-timer.c 			applet-timer.h
	applet-theme.c 			applet-theme.h
	applet-calendar.c 		applet-calendar.h
	applet-task-editor.c 		applet-task-editor.h
	applet-backend-default.c 	applet-backend-default.h
	applet-backend-ical.c 	applet-backend-ical.h
)

add_library(${PACKAGE_CLOCK} SHARED ${cd-clock_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${clockdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="clock.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="${clockuserdirname}")
add_definitions (-DMY_APPLET_VERSION="${VERSION_CLOCK}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_CLOCK}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")
if (NOT LIBICAL_PACKAGE_FOUND)
	add_definitions (-DCD_CLOCK_ICAL_SUPPORT="1")
endif()

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${LIBICAL_PACKAGE_INCLUDE_DIRS} "/usr/include/libical")

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${LIBICAL_PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_CLOCK}
	${PACKAGE_LIBRARIES}
	${LIBICAL_PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_CLOCK} DESTINATION ${pluginsdir})
