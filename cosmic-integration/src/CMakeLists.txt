
########### sources ###############

SET(cd_cosmic-integration_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-utils.c
	applet-utils.h
)

add_library(${PACKAGE_COSMIC_INTEGRATION} SHARED ${cd_cosmic-integration_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${cosmic_integrationdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="none")
add_definitions (-DMY_APPLET_USER_DATA_DIR="cosmic-integration")
add_definitions (-DMY_APPLET_VERSION="${VERSION_COSMIC_INTEGRATION}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_COSMIC_INTEGRATION}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")

include_directories (${PACKAGE_INCLUDE_DIRS})

link_directories (${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_COSMIC_INTEGRATION}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_COSMIC_INTEGRATION} DESTINATION ${pluginsdir})
