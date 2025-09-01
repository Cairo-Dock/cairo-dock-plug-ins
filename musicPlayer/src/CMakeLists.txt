
########### sources ###############

SET(cd-musicPlayer_LIB_SRCS
	applet-struct.h
	applet-init.c			applet-init.h
	applet-config.c			applet-config.h
	applet-notifications.c		applet-notifications.h
	applet-dbus.c			applet-dbus.h
	applet-musicplayer.c		applet-musicplayer.h
	applet-cover.c			applet-cover.h
	applet-amazon.c			applet-amazon.h
	applet-draw.c			applet-draw.h
	3dcover-draw.c			3dcover-draw.h
	# backends
	applet-mpris.c			applet-mpris.h
	applet-mpris2.c			applet-mpris2.h
	applet-amarok2.c			applet-amarok2.h
	applet-clementine.c		applet-clementine.h
	applet-exaile3.c			applet-exaile3.h
	applet-gmusicbrowser.c		applet-gmusicbrowser.h
	applet-guayadeque.c		applet-guayadeque.h
	applet-qmmp.c			applet-qmmp.h
	applet-quodlibet.c		applet-quodlibet.h
	applet-rhythmbox.c		applet-rhythmbox.h
	applet-audacious.c		applet-audacious.h
	applet-xmms2.c			applet-xmms2.h
)

add_library(${PACKAGE_MUSICPLAYER} SHARED ${cd-musicPlayer_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${musicplayerdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="musicPlayer.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="musicPlayer")
add_definitions (-DMY_APPLET_VERSION="${VERSION_MUSICPLAYER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_MUSICPLAYER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_MUSICPLAYER}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_MUSICPLAYER} DESTINATION ${pluginsdir})
