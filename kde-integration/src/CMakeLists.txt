
########### sources ###############

SET(cd_kde-integration_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-vfs.c
	applet-vfs.h
	applet-utils.c
	applet-utils.h
)

add_library(${PACKAGE_KDE_INTEGRATION} SHARED ${cd_kde-integration_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(kde_integrationdatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="none")
add_definitions (-DMY_APPLET_USER_DATA_DIR="kde-integration")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_KDE_INTEGRATION)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="cd_kde-integration")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS}
	${KDE_INTEGRATION_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS}
	${KDE_INTEGRATION_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_KDE_INTEGRATION}
	${PACKAGE_LIBRARIES}
	${KDE_INTEGRATION_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_KDE_INTEGRATION} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd_kde-integration.la
#
#
#libcd_kde_integration_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-vfs.c
#	applet-vfs.h
#	applet-utils.c
#	applet-utils.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_kde_integration_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	$(KDE_INTEGRATION_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(kde_integrationdatadir)""
#	-DMY_APPLET_PREVIEW_FILE="none"""
#	-DMY_APPLET_CONF_FILE=""none""
#	-DMY_APPLET_USER_DATA_DIR=""kde-integration""
#	-DMY_APPLET_VERSION=""$(VERSION_KDE_INTEGRATION)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""cd_kde-integration""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_kde_integration_la_LIBADD = 
#	$(PACKAGE_LIBS)
#	$(KDE_INTEGRATION_LIBS)
#	-lm
#
#
#libcd_kde_integration_la_LDFLAGS =
#	-avoid-version -module
