
########### sources ###############

SET(cd_xfce-integration_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-thunar-vfs.h
	applet-utils.c
	applet-utils.h
)
if(${XFCE_INTEGRATION_GVFS_FOUND})
	MESSAGE("    --> gvfs found: source file applet-thunar-vfs.c NOT included for compilation.")
else()
	LIST(APPEND cd_xfce-integration_LIB_SRCS applet-thunar-vfs.c)
	add_definitions (-DHAVE_THUNAR="1")
	MESSAGE("    --> gvfs not found: source file applet-thunar-vfs.c included for compilation.")
endif()

add_library(${PACKAGE_XFCE_INTEGRATION} SHARED ${cd_xfce-integration_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${xfce_integration}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="none")
add_definitions (-DMY_APPLET_USER_DATA_DIR="xfce-integration")
add_definitions (-DMY_APPLET_VERSION="${VERSION_XFCE_INTEGRATION}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="cd_xfce-integration")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${XFCE_INTEGRATION_THUNAR_INCLUDE_DIRS}
	${XFCE_INTEGRATION_GVFS_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${XFCE_INTEGRATION_THUNAR_LIBRARY_DIRS}
	${XFCE_INTEGRATION_GVFS_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_XFCE_INTEGRATION}
	${PACKAGE_LIBRARIES}
	${XFCE_INTEGRATION_THUNAR_LIBRARIES}
	${XFCE_INTEGRATION_GVFS_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_XFCE_INTEGRATION} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd_xfce-integration.la
#
#
#libcd_xfce_integration_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-thunar-vfs.c
#	applet-thunar-vfs.h
#	applet-utils.c
#	applet-utils.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_xfce_integration_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	$(XFCE_INTEGRATION_CFLAGS)
#add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${xfce_integration)"
#add_definitions (-DMY_APPLET_PREVIEW_FILE="none""
#add_definitions (-DMY_APPLET_CONF_FILE="none"
#add_definitions (-DMY_APPLET_USER_DATA_DIR="xfce-integration"
#add_definitions (-DMY_APPLET_VERSION="${VERSION_XFCE_INTEGRATION)"
#add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="cd_xfce-integration"
#add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version)"
#add_definitions (-DMY_APPLET_ICON_FILE="icon.png"
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_xfce_integration_la_LIBADD = 
#	$(PACKAGE_LIBS)
#	$(XFCE_INTEGRATION_LIBS)
#	-lm
#
#
#libcd_xfce_integration_la_LDFLAGS =
#	-avoid-version -module
