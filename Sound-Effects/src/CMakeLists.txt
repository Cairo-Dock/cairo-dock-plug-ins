
########### sources ###############

SET(MODULE_SRCS
	applet-struct.h
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-sound.c 				applet-sound.h
	applet-notifications.c 		applet-notifications.h
)

add_library(${PACKAGE_SOUND_EFFECTS} SHARED ${MODULE_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${sound_effectsdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.png")
add_definitions (-DMY_APPLET_CONF_FILE="Sound-Effects.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Sound-Effects")
add_definitions (-DMY_APPLET_VERSION="${VERSION_SOUND_EFFECTS}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_SOUND_EFFECTS}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
### uncomment the following line to allow multi-instance applet.
#add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
### uncomment the following line to allow extended OpenGL drawing.
#add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${ALSA_PACKAGE_INCLUDE_DIRS}
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${ALSA_PACKAGE_LIBRARY_DIRS}
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_SOUND_EFFECTS}
	${ALSA_PACKAGE_LIBRARIES}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_SOUND_EFFECTS} DESTINATION ${pluginsdir})
