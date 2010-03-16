
########### sources ###############

SET(cd-show_mouse_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
	star-tex.h
)

add_library(${PACKAGE_SHOW_MOUSE} SHARED ${cd-show_mouse_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(show_mousedatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="show_mouse.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="show_mouse")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_SHOW_MOUSE)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_SHOW_MOUSE)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_SHOW_MOUSE}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_SHOW_MOUSE} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-show_mouse.la
#
#
#libcd_show_mouse_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-struct.h
#	star-tex.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_show_mouse_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(show_mousedatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""show_mouse.conf""
#	-DMY_APPLET_USER_DATA_DIR=""show_mouse""
#	-DMY_APPLET_VERSION=""$(VERSION_SHOW_MOUSE)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_SHOW_MOUSE)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#     -DGL_GLEXT_PROTOTYPES=""1""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_show_mouse_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_show_mouse_la_LDFLAGS =
#	-avoid-version -module
