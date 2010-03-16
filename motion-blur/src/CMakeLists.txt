
########### sources ###############

SET(cd-motion_blur_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
)

add_library(${PACKAGE_MOTION_BLUR} SHARED ${cd-motion_blur_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(motion_blurdatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="motion_blur.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="motion_blur")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_MOTION_BLUR)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_MOTION_BLUR)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_MOTION_BLUR}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_MOTION_BLUR} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-motion_blur.la
#
#
#libcd_motion_blur_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_motion_blur_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(motion_blurdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""none""
#	-DMY_APPLET_CONF_FILE=""motion_blur.conf""
#	-DMY_APPLET_USER_DATA_DIR=""motion_blur""
#	-DMY_APPLET_VERSION=""$(VERSION_MOTION_BLUR)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_MOTION_BLUR)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_motion_blur_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_motion_blur_la_LDFLAGS =
#	-avoid-version -module
