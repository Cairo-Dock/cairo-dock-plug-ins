
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-notifications.c 		applet-notifications.h
	applet-indicator3.c			applet-indicator3.h
)

add_library(${PACKAGE_PRINTERS_MENU} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${printers_menudatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Printers-Menu.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Printers-Menu")
add_definitions (-DMY_APPLET_VERSION="${VERSION_PRINTERS_MENU}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_PRINTERS_MENU}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${INDICATOR_APPLET_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/Indicator-applet3)

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${INDICATOR_APPLET_LIBRARY_DIRS}
	${CMAKE_SOURCE_DIR}/Indicator-applet3)

target_link_libraries (${PACKAGE_PRINTERS_MENU}
	${PACKAGE_LIBRARIES}
	${INDICATOR_APPLET_LIBRARIES}
	indicator-applet3)

########### install files ###############

install(TARGETS ${PACKAGE_PRINTERS_MENU} DESTINATION ${pluginsdir})
