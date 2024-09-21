
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-notifications.c 		applet-notifications.h
	applet-indicator3.c 		applet-indicator3.h
	applet-launcher.c 			applet-launcher.h
)

add_library(${PACKAGE_INDICATOR_GENERIC} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${indicator_genericdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Indicator-Generic.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Indicator-Generic")
add_definitions (-DMY_APPLET_VERSION="${VERSION_INDICATOR_GENERIC}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_INDICATOR_GENERIC}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
### uncomment the following line to allow multi-instance applet.
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
### uncomment the following line to allow extended OpenGL drawing.
#add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${INDICATOR_APPLET_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/Indicator-applet3)

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${INDICATOR_APPLET_LIBRARY_DIRS}
	${CMAKE_SOURCE_DIR}/Indicator-applet3)

target_link_libraries (${PACKAGE_INDICATOR_GENERIC}
	${PACKAGE_LIBRARIES}
	${INDICATOR_APPLET_LIBRARIES}
	indicator-applet3)

########### install files ###############

install(TARGETS ${PACKAGE_INDICATOR_GENERIC} DESTINATION ${pluginsdir})
