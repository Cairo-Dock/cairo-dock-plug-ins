
########### sources ###############

SET(cd-StatusNotifier_LIB_SRCS
	applet-init.c 		applet-init.h
	applet-config.c 		applet-config.h
	applet-host.c 		applet-host.h
	applet-item.c 		applet-item.h
	applet-notifications.c 	applet-notifications.h
	applet-struct.h
)

add_library(${PACKAGE_STATUS_NOTIFIER} SHARED ${cd-StatusNotifier_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${status_notifierdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Status-Notifier.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Status-Notifier")
add_definitions (-DMY_APPLET_VERSION="${VERSION_STATUSNOTIFIER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_STATUSNOTIFIER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${STATUS_NOTIFIER_PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${STATUS_NOTIFIER_PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_STATUS_NOTIFIER}
	${PACKAGE_LIBRARIES}
	${STATUS_NOTIFIER_PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_STATUS_NOTIFIER} DESTINATION ${pluginsdir})
