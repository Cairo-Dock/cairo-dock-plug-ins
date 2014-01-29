
########### sources ###############

SET(cd-GMenu_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-menu-callbacks.c
	applet-menu-callbacks.h
	applet-menu.c
	applet-menu.h
	applet-util.c
	applet-util.h
	applet-recent.c
	applet-recent.h
	applet-run-dialog.c
	applet-run-dialog.h
	applet-struct.h
)

add_library(${PACKAGE_GMENU} SHARED ${cd-GMenu_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${gmenudatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="GMenu.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="GMenu")
add_definitions (-DMY_APPLET_VERSION="${VERSION_GMENU}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_GMENU}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
if (${LIBGIO_FOUND})
	add_definitions (-DHAVE_GIO=1)
endif()

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${GMENU_PACKAGE_INCLUDE_DIRS}
	${LIBGIO_PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${GMENU_PACKAGE_LIBRARY_DIRS}
	${LIBGIO_PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_GMENU}
	${PACKAGE_LIBRARIES}
	${GMENU_PACKAGE_LIBRARIES}
	${LIBGIO_PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_GMENU} DESTINATION ${pluginsdir})

