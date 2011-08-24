
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-load-icons.c 		applet-load-icons.h
	applet-notifications.c 		applet-notifications.h
)

add_library(${PACKAGE_FOLDERS} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${foldersdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Folders.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Folders")
add_definitions (-DMY_APPLET_VERSION="${VERSION_FOLDERS}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_FOLDERS}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
### uncomment the following line to allow extended OpenGL drawing.
#add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_FOLDERS}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_FOLDERS} DESTINATION ${pluginsdir})
