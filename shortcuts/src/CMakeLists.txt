
########### sources ###############

SET(cd-shortcuts_LIB_SRCS
	applet-init.c 		applet-init.h
	applet-config.c 		applet-config.h
	applet-notifications.c 	applet-notifications.h
	applet-bookmarks.c 	applet-bookmarks.h
	applet-load-icons.c 	applet-load-icons.h
	applet-disk-usage.c 	applet-disk-usage.h
	applet-drives.c 		applet-drives.h
	applet-struct.h
)

add_library(${PACKAGE_SHORTCUTS} SHARED ${cd-shortcuts_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${shortcutsdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="shortcuts.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="shortcuts")
add_definitions (-DMY_APPLET_VERSION="${VERSION_SHORTCUTS}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_SHORTCUTS}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_SHORTCUTS}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_SHORTCUTS} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-shortcuts.la
#
#
#libcd_shortcuts_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-bookmarks.c
#	applet-bookmarks.h
#	applet-load-icons.c
#	applet-load-icons.h
#	applet-disk-usage.c
#	applet-disk-usage.h
#	applet-struct.h
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_shortcuts_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${shortcutsdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""shortcuts.conf""
#	-DMY_APPLET_USER_DATA_DIR=""shortcuts""
#	-DMY_APPLET_VERSION=""${VERSION_SHORTCUTS)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_SHORTCUTS)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-DCD_APPLET_MULTI_INSTANCE="1"
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_shortcuts_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_shortcuts_la_LDFLAGS =
#	-avoid-version -module
