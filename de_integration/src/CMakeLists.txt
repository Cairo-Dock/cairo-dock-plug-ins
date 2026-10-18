
########### sources ###############

SET(cd_de-integration_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-kde-init.c
	applet-kde-vfs.c
	applet-kde-utils.c
	applet-kde.h
	applet-gnome.c
	applet-cosmic.c
	applet-xfce.c
)

add_library(${PACKAGE_DE_INTEGRATION} SHARED ${cd_de-integration_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${de_integrationdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="de_integration.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="de-integration")
add_definitions (-DMY_APPLET_VERSION="${VERSION_DE_INTEGRATION}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_DE_INTEGRATION}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (${PACKAGE_INCLUDE_DIRS})

link_directories (${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_DE_INTEGRATION}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_DE_INTEGRATION} DESTINATION ${pluginsdir})

