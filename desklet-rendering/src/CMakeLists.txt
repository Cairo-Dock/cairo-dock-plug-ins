
########### sources ###############

SET(cd-desklet-rendering_LIB_SRCS
	rendering-init.c 				rendering-init.h
	rendering-config.c 				rendering-config.h
	rendering-desklet-caroussel.c 	rendering-desklet-caroussel.h
	rendering-desklet-tree.c 		rendering-desklet-tree.h
	rendering-desklet-simple.c 		rendering-desklet-simple.h
	rendering-desklet-decorations.c rendering-desklet-decorations.h
	rendering-desklet-slide.c 		rendering-desklet-slide.h
	rendering-desklet-viewport.c 		rendering-desklet-viewport.h
	rendering-desklet-panel.c 		rendering-desklet-panel.h
	rendering-struct.h
)

add_library(${PACKAGE_DESKLET_RENDERING} SHARED ${cd-desklet-rendering_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${desklet_renderingdatadir}")
add_definitions (-DMY_APPLET_VERSION="${VERSION_DESKLET_RENDERING}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_DESKLET_RENDERING}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="none")
add_definitions (-DMY_APPLET_USER_DATA_DIR="desklet-rendering")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_DESKLET_RENDERING}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_DESKLET_RENDERING} DESTINATION ${pluginsdir})
