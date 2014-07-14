
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
if (INDICATOR_SOUNDMENU_WITH_IND3)
	list (APPEND cd-AlsaMixer_LIB_SRCS
		applet-backend-sound-menu.c applet-backend-sound-menu.h)
elseif (with_soundmenu)
	list (APPEND cd-AlsaMixer_LIB_SRCS
		applet-backend-sound-menu-old.c applet-backend-sound-menu-old.h
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

if (INDICATOR_SOUNDMENU_WITH_IND3)
	message (STATUS "   With Indicator-Applet3")
	add_definitions (-DINDICATOR_SOUNDMENU_WITH_IND3=1)
	if (INDICATOR_NG)
		add_definitions (-DIS_INDICATOR_NG=1)
	endif()
	set (EXTRAS_INCLUDE_DIRS ${INDICATOR_APPLET_INCLUDE_DIRS}
		${CMAKE_SOURCE_DIR}/Indicator-applet3)
	set (EXTRAS_LIBRARY_DIRS ${INDICATOR_APPLET_LIBRARY_DIRS}
		${CMAKE_SOURCE_DIR}/Indicator-applet3)
	set (EXTRAS_LIBRARIES ${INDICATOR_APPLET_LIBRARIES} indicator-applet3)
elseif (with_soundmenu)
	add_definitions (-DSOUND_SERVICE_SUPPORT="1")
	add_definitions (-DSOUND_SERVICE_VERSION=${SOUND_SERVICE_VERSION})
	set (EXTRAS_INCLUDE_DIRS ${DBUSMENU_INCLUDE_DIRS}
		${DBUSMENU_GTK_INCLUDE_DIRS} ${INDICATOR_APPLET_INCLUDE_DIRS}
		${IDO_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/Indicator-applet)
	set (EXTRAS_LIBRARY_DIRS ${DBUSMENU_LIBRARY_DIRS}
		${DBUSMENU_GTK_LIBRARY_DIRS} ${INDICATOR_APPLET_LIBRARY_DIRS}
		${IDO_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/Indicator-applet)
	set (EXTRAS_LIBRARIES ${DBUSMENU_LIBRARIES} ${DBUSMENU_GTK_LIBRARIES}
		${INDICATOR_APPLET_LIBRARIES} ${IDO_LIBRARIES} indicator-applet)
endif()

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${ALSA_MIXER_PACKAGE_INCLUDE_DIRS}
	${EXTRAS_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${ALSA_MIXER_PACKAGE_LIBRARY_DIRS}
	${EXTRAS_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_ALSA_MIXER}
	${PACKAGE_LIBRARIES}
	${ALSA_MIXER_PACKAGE_LIBRARIES}
	${EXTRAS_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_ALSA_MIXER} DESTINATION ${pluginsdir})

