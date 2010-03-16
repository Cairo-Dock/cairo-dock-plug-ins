
########### sources ###############

SET(cd_gnome-integration-old_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-gnome-vfs.c
	applet-gnome-vfs.h
	applet-utils.c
	applet-utils.h
)

add_library(${PACKAGE_GNOME_INTEGRATION_OLD} SHARED ${cd_gnome-integration-old_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${gnome_integration_olddatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="none")
add_definitions (-DMY_APPLET_USER_DATA_DIR="gnome-integration-old")
add_definitions (-DMY_APPLET_VERSION="${VERSION_GNOME_INTEGRATION_OLD}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="cd_gnome-integration-old")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${OLD_GNOME_INTEGRATION_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${OLD_GNOME_INTEGRATION_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_GNOME_INTEGRATION_OLD}
	${PACKAGE_LIBRARIES}
	${OLD_GNOME_INTEGRATION_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_GNOME_INTEGRATION_OLD} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd_gnome-integration-old.la
#
#
#libcd_gnome_integration_old_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-gnome-vfs.c
#	applet-gnome-vfs.h
#	applet-utils.c
#	applet-utils.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_gnome_integration_old_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	$(OLD_GNOME_INTEGRATION_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${gnome_integration_olddatadir)""
#	-DMY_APPLET_PREVIEW_FILE="none"""
#	-DMY_APPLET_CONF_FILE=""none""
#	-DMY_APPLET_USER_DATA_DIR=""gnome-integration-old""
#	-DMY_APPLET_VERSION=""${VERSION_GNOME_INTEGRATION_OLD)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""cd_gnome-integration-old""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.svg""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_gnome_integration_old_la_LIBADD = 
#	$(PACKAGE_LIBS)
#	$(OLD_GNOME_INTEGRATION_LIBS)
#	-lm
#
#
#libcd_gnome_integration_old_la_LDFLAGS =
#	-avoid-version -module
