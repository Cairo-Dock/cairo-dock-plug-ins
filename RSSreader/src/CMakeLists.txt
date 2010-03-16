
########### sources ###############

SET(cd-RSSreader_LIB_SRCS
	applet-init.c 		applet-init.h
	applet-config.c 		applet-config.h
	applet-notifications.c 	applet-notifications.h
	applet-draw.c 		applet-draw.h
	applet-rss.c 		applet-rss.h
	applet-struct.h
)

add_library(${PACKAGE_RSS_READER} SHARED ${cd-RSSreader_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="$(rss_readerdatadir)")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="RSSreader.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="RSSreader")
add_definitions (-DMY_APPLET_VERSION="$(VERSION_RSS_READER)")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="$(GETTEXT_RSS_READER)")
add_definitions (-DMY_APPLET_DOCK_VERSION="$(dock_version)")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_RSS_READER}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_RSS_READER} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-RSSreader.la
#
#
#libcd_RSSreader_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-draw.c
#	applet-draw.h
#	applet-rss.c
#	applet-rss.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_RSSreader_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""$(rss_readerdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""RSSreader.conf""
#	-DMY_APPLET_USER_DATA_DIR=""RSSreader""
#	-DMY_APPLET_VERSION=""$(VERSION_RSS_READER)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""$(GETTEXT_RSS_READER)""
#	-DMY_APPLET_DOCK_VERSION=""$(dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.svg""
#	-DCD_APPLET_MULTI_INSTANCE="1"
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_RSSreader_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_RSSreader_la_LDFLAGS =
#	-avoid-version -module
