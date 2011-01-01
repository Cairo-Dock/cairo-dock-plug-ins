
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 			applet-init.h
	applet-config.c 			applet-config.h
	applet-search.c 			applet-search.h
	applet-notifications.c 		applet-notifications.h
)

add_library(${PACKAGE_RECENT_EVENTS} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${recent_eventsdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Recent-Events.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Recent-Events")
add_definitions (-DMY_APPLET_VERSION="${VERSION_RECENT_EVENTS}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_RECENT_EVENTS}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
### uncomment the following line to allow multi-instance applet.
#add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
### uncomment the following line to allow extended OpenGL drawing.
#add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${RECENT_EVENTS_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${RECENT_EVENTS_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_RECENT_EVENTS}
	${PACKAGE_LIBRARIES}
	${RECENT_EVENTS_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_RECENT_EVENTS} DESTINATION ${pluginsdir})
