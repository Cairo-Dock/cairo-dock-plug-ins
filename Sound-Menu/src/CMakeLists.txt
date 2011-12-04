
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 			applet-init.h
	applet-config.c 		applet-config.h
	applet-sound.c 			applet-sound.h
	applet-menu.c 			applet-menu.h
	applet-notifications.c 	applet-notifications.h
	volume-widget.c			volume-widget.h
	voip-input-widget.c		voip-input-widget.h
	transport-widget.c		transport-widget.h
	title-widget.c			title-widget.h
	metadata-widget.c		metadata-widget.h
	dbus-shared-names.h
)

add_library(${PACKAGE_SOUND_MENU} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${sound_menudatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Sound-Menu.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Sound-Menu")
add_definitions (-DMY_APPLET_VERSION="${VERSION_SOUND_MENU}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_SOUND_MENU}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DINDICATOR_OLD_NAMES=${INDICATOR_OLD_NAMES})
### uncomment the following line to allow multi-instance applet.
#add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
### uncomment the following line to allow extended OpenGL drawing.
#add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${DBUSMENU_INCLUDE_DIRS}
	${DBUSMENU_GTK_INCLUDE_DIRS}
	${INDICATOR_APPLET_INCLUDE_DIRS}
	${IDO_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/Indicator-applet)

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${DBUSMENU_LIBRARY_DIRS}
	${DBUSMENU_GTK_LIBRARY_DIRS}
	${INDICATOR_APPLET_LIBRARY_DIRS}
	${IDO_LIBRARY_DIRS}
	${CMAKE_SOURCE_DIR}/Indicator-applet)

target_link_libraries (${PACKAGE_SOUND_MENU}
	${PACKAGE_LIBRARIES}
	${DBUSMENU_LIBRARIES}
	${DBUSMENU_GTK_LIBRARIES}
	${INDICATOR_APPLET_LIBRARIES}
	${IDO_LIBRARIES}
	indicator-applet)
########### install files ###############

install(TARGETS ${PACKAGE_SOUND_MENU} DESTINATION ${pluginsdir})
