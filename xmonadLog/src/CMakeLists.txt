
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-notifications.c 		applet-notifications.h
)

add_library(${PACKAGE_XMONADLOG} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${xmonadlogdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="xmonadLog.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="xmonadLog")
add_definitions (-DMY_APPLET_VERSION="${VERSION_XMONADLOG}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_XMONADLOG}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
### uncomment the following line to allow multi-instance applet.
#add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
### uncomment the following line to allow extended OpenGL drawing.
#add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_XMONADLOG}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_XMONADLOG} DESTINATION ${pluginsdir})
