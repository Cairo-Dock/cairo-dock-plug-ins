
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-notifications.c 		applet-notifications.h
	applet-app.c 				applet-app.h
	applet-draw.c 				applet-draw.h
)

add_library(${PACKAGE_GLOBAL_MENU} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${global_menudatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Global-Menu.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Global-Menu")
add_definitions (-DMY_APPLET_VERSION="${VERSION_GLOBAL_MENU}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_GLOBAL_MENU}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DCD_PLUGINS_DIR="${pluginsdir}")
### uncomment the following line to allow multi-instance applet.
#add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
### uncomment the following line to allow extended OpenGL drawing.
#add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${DBUSMENU_INCLUDE_DIRS}
	${DBUSMENU_GTK_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${DBUSMENU_LIBRARY_DIRS}
	${DBUSMENU_GTK_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_GLOBAL_MENU}
	${PACKAGE_LIBRARIES}
	${DBUSMENU_LIBRARIES}
	${DBUSMENU_GTK_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_GLOBAL_MENU} DESTINATION ${pluginsdir})
