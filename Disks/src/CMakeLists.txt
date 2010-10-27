
########### sources ###############

SET(cd-disks_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-disks.c
	applet-disks.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
)

add_library(${PACKAGE_DISKS} SHARED ${cd-disks_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${disksdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Disks.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Disks")
add_definitions (-DMY_APPLET_VERSION="${VERSION_DISKS}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_DISKS}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_DISKS}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_DISKS} DESTINATION ${pluginsdir})
