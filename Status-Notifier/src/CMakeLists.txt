
########### sources ###############

SET(cd-StatusNotifier_LIB_SRCS
	applet-init.c 		applet-init.h
	applet-config.c 		applet-config.h
	applet-host.c 		applet-host.h
	applet-host-kde.c 	applet-host-kde.h
	applet-host-ias.c 	applet-host-ias.h
	applet-item.c 		applet-item.h
	applet-draw.c 		applet-draw.h
	applet-notifications.c 	applet-notifications.h
	applet-struct.h
)

add_library(${PACKAGE_STATUS_NOTIFIER} SHARED ${cd-StatusNotifier_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${status_notifierdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Status-Notifier.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Status-Notifier")
add_definitions (-DMY_APPLET_VERSION="${VERSION_STATUS_NOTIFIER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_STATUS_NOTIFIER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DINDICATOR_OLD_NAMES=${INDICATOR_OLD_NAMES})
add_definitions (-DINDICATOR_APPLICATIONADDED_HAS_HINT=${INDICATOR_APPLICATIONADDED_HAS_HINT})
add_definitions (-DINDICATOR_APPLICATIONADDED_HAS_TITLE=${INDICATOR_APPLICATIONADDED_HAS_TITLE})
add_definitions (-DCD_PLUGINS_DIR="${pluginsdir}")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${DBUSMENU_GTK_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${DBUSMENU_GTK_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_STATUS_NOTIFIER}
	${PACKAGE_LIBRARIES}
	${DBUSMENU_GTK_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_STATUS_NOTIFIER} DESTINATION ${pluginsdir})
