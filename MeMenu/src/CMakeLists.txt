
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 			applet-init.h
	applet-config.c 			applet-config.h
	applet-me.c 			applet-me.h
	applet-menu.c 			applet-menu.h
	about-me-menu-item.c		about-me-menu-item.h
	applet-notifications.c 		applet-notifications.h
)

add_library(${PACKAGE_MEMENU} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${memenudatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="MeMenu.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="MeMenu")
add_definitions (-DMY_APPLET_VERSION="${VERSION_MEMENU}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_MEMENU}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DDBUSMENU_MAJOR=${DBUSMENU_MAJOR})
add_definitions (-DDBUSMENU_MINOR=${DBUSMENU_MINOR})
add_definitions (-DDBUSMENU_NANO=${DBUSMENU_NANO})
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
	${CMAKE_SOURCE_DIR}/Indicator-applet)

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${DBUSMENU_LIBRARY_DIRS}
	${DBUSMENU_GTK_LIBRARY_DIRS}
	${INDICATOR_APPLET_LIBRARY_DIRS}
	${CMAKE_SOURCE_DIR}/Indicator-applet)

target_link_libraries (${PACKAGE_MEMENU}
	${PACKAGE_LIBRARIES}
	${DBUSMENU_LIBRARIES}
	${DBUSMENU_GTK_LIBRARIES}
	${INDICATOR_APPLET_LIBRARIES}
	indicator-applet)

########### install files ###############

install(TARGETS ${PACKAGE_MEMENU} DESTINATION ${pluginsdir})
