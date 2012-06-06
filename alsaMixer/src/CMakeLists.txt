
########### sources ###############

set (cd-AlsaMixer_LIB_SRCS
	applet-init.c 				applet-init.h
	applet-config.c 			applet-config.h
	applet-notifications.c 		applet-notifications.h
	applet-draw.c 				applet-draw.h
	applet-generic.c 			applet-generic.h
	# alsa backend
	applet-backend-alsamixer.c 	applet-backend-alsamixer.h
	applet-struct.h)

# Sound service backend (optionnal)
if ("${with_soundmenu}" STREQUAL "yes")
	set (cd-AlsaMixer_LIB_SRCS
		${cd-AlsaMixer_LIB_SRCS}
		applet-backend-sound-menu.c applet-backend-sound-menu.h
		applet-menu.c 				applet-menu.h
		volume-widget.c				volume-widget.h
		voip-input-widget.c			voip-input-widget.h
		transport-widget.c			transport-widget.h
		mute-widget.c				mute-widget.h
		metadata-widget.c			metadata-widget.h)
endif()

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
if ("${with_soundmenu}" STREQUAL "yes")
	add_definitions (-DSOUND_SERVICE_SUPPORT="1")
	add_definitions (-DSOUND_SERVICE_VERSION=${SOUND_SERVICE_VERSION})
endif()
if (${DBUSMENU_GTK3_NEW})
	add_definitions (-DDBUSMENU_GTK3_NEW=1)
endif()


if ("${with_soundmenu}" STREQUAL "yes")
	include_directories (
		${PACKAGE_INCLUDE_DIRS}
		${ALSA_MIXER_PACKAGE_INCLUDE_DIRS}
		${DBUSMENU_INCLUDE_DIRS}
		${DBUSMENU_GTK_INCLUDE_DIRS}
		${INDICATOR_APPLET_INCLUDE_DIRS}
		${IDO_INCLUDE_DIRS}
		${CMAKE_SOURCE_DIR}/Indicator-applet)

	link_directories (
		${PACKAGE_LIBRARY_DIRS}
		${ALSA_MIXER_PACKAGE_LIBRARY_DIRS}
		${DBUSMENU_LIBRARY_DIRS}
		${DBUSMENU_GTK_LIBRARY_DIRS}
		${INDICATOR_APPLET_LIBRARY_DIRS}
		${IDO_LIBRARY_DIRS}
		${CMAKE_SOURCE_DIR}/Indicator-applet)

	target_link_libraries (${PACKAGE_ALSA_MIXER}
		${PACKAGE_LIBRARIES}
		${ALSA_MIXER_PACKAGE_LIBRARIES}
		${DBUSMENU_LIBRARIES}
		${DBUSMENU_GTK_LIBRARIES}
		${INDICATOR_APPLET_LIBRARIES}
		${IDO_LIBRARIES}
		indicator-applet)
else()
	include_directories (
		${PACKAGE_INCLUDE_DIRS}
		${ALSA_MIXER_PACKAGE_INCLUDE_DIRS})

	link_directories (
		${PACKAGE_LIBRARY_DIRS}
		${ALSA_MIXER_PACKAGE_LIBRARY_DIRS})

	target_link_libraries (${PACKAGE_ALSA_MIXER}
		${PACKAGE_LIBRARIES}
		${ALSA_MIXER_PACKAGE_LIBRARIES})
endif()

########### install files ###############

install(TARGETS ${PACKAGE_ALSA_MIXER} DESTINATION ${pluginsdir})

