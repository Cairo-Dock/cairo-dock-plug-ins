
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-composite-manager.c	applet-composite-manager.h
	applet-notifications.c 		applet-notifications.h
)

add_library(${PACKAGE_COMPOSITE_MANAGER} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${composite_managerdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Composite-Manager.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Composite-Manager")
add_definitions (-DMY_APPLET_VERSION="${VERSION_COMPOSITE_MANAGER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_COMPOSITE_MANAGER}")
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

target_link_libraries (${PACKAGE_COMPOSITE_MANAGER}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_COMPOSITE_MANAGER} DESTINATION ${pluginsdir})
