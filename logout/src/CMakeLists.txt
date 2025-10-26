
########### sources ###############

SET(cd-logout_LIB_SRCS
	applet-init.c				applet-init.h
	applet-config.c				applet-config.h
	applet-notifications.c		applet-notifications.h
	applet-logout.c				applet-logout.h
	applet-reboot-required.c	applet-reboot-required.h
	applet-timer.c				applet-timer.h
	applet-struct.h
)

add_library(${PACKAGE_LOGOUT} SHARED ${cd-logout_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${logoutdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="logout.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="logout")
add_definitions (-DMY_APPLET_VERSION="${VERSION_LOGOUT}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_LOGOUT}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
if (DEBIAN_PKG_MANAGERS)
	add_definitions (-DEND_INSTALLATION_PID="${DEBIAN_PKG_MANAGERS}")
endif()

include_directories (${PACKAGE_INCLUDE_DIRS})

link_directories (${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_LOGOUT}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_LOGOUT} DESTINATION ${pluginsdir})
