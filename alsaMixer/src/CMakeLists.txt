
########### sources ###############

set (cd-AlsaMixer_LIB_SRCS
	applet-init.c 			applet-init.h
	applet-config.c 		applet-config.h
	applet-notifications.c 	applet-notifications.h
	applet-mixer.c 			applet-mixer.h
	applet-draw.c 			applet-draw.h
	applet-struct.h)

add_library (${PACKAGE_ALSA_MIXER} SHARED ${cd-AlsaMixer_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${alsa_mixerdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="AlsaMixer.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="AlsaMixer")
add_definitions (-DMY_APPLET_VERSION="${VERSION_ALSA_MIXER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_ALSA_MIXER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${ALSA_MIXER_PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${ALSA_MIXER_PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_ALSA_MIXER}
	${PACKAGE_LIBRARIES}
	${ALSA_MIXER_PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_ALSA_MIXER} DESTINATION ${pluginsdir})
