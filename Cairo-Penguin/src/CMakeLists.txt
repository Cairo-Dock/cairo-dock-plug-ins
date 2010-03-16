
########### next target ###############

SET(cd-Cairo-Penguin_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-notifications.c
	applet-notifications.h
	applet-theme.c
	applet-theme.h
	applet-animation.c
	applet-animation.h
	applet-struct.h
)

add_library(${PACKAGE_CAIRO_PENGUIN} SHARED ${cd-Cairo-Penguin_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(cairo_penguindatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Cairo-Penguin.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="$(cairo_penguinuserdirname)")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_CAIRO_PENGUIN)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_CAIRO_PENGUIN)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_CAIRO_PENGUIN}
	${PACKAGE_LIBRARIES})

install(TARGETS ${PACKAGE_CAIRO_PENGUIN} DESTINATION ${pluginsdir})



#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-Cairo-Penguin.la
#
#
#libcd_Cairo_Penguin_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-theme.c
#	applet-theme.h
#	applet-animation.c
#	applet-animation.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_Cairo_Penguin_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(cairo_penguindatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""Cairo-Penguin.conf""
#	-DMY_APPLET_USER_DATA_DIR=""$(cairo_penguinuserdirname)""
#	-DMY_APPLET_VERSION=""$(VERSION_CAIRO_PENGUIN)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_CAIRO_PENGUIN)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-DCD_APPLET_MULTI_INSTANCE="1"
#	-DGL_GLEXT_PROTOTYPES=""1""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_Cairo_Penguin_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_Cairo_Penguin_la_LDFLAGS =
#	-avoid-version -module
