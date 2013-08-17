
########### next target ###############

SET(cd-Animated-icons_LIB_SRCS
	applet-init.c 			applet-init.h
	applet-config.c 		applet-config.h
	applet-rotation.c 		applet-rotation.h
	applet-mesh-factory.c 	applet-mesh-factory.h
	applet-notifications.c 	applet-notifications.h
	applet-spot.c 			applet-spot.h
	applet-rays.c 			applet-rays.h
	applet-wobbly.c 		applet-wobbly.h
	applet-wave.c 			applet-wave.h
	applet-pulse.c 			applet-pulse.h
	applet-bounce.c 		applet-bounce.h
	applet-blink.c 			applet-blink.h
	applet-busy.c 			applet-busy.h
	applet-unfold.c 		applet-unfold.h
	applet-struct.h
	chrome-tex.h
)

add_library(${PACKAGE_ANIMATED_ICONS} SHARED ${cd-Animated-icons_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${animated_iconsdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Animated-icons.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Animated-icons")
add_definitions (-DMY_APPLET_VERSION="${VERSION_ANIMATED_ICONS}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_ANIMATED_ICONS}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_ANIMATED_ICONS}
	${PACKAGE_LIBRARIES})

install(TARGETS ${PACKAGE_ANIMATED_ICONS} DESTINATION ${pluginsdir})
