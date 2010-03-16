
########### sources ###############

SET(cd-netspeed_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-netspeed.c
	applet-netspeed.h
	applet-notifications.c
	applet-notifications.h
	applet-struct.h
)

add_library(${PACKAGE_NETSPEED} SHARED ${cd-netspeed_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${netspeeddatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="netspeed.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="netspeed")
add_definitions (-DMY_APPLET_VERSION="${VERSION_NETSPEED}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_NETSPEED}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.png")
add_definitions (-DCD_APPLET_MULTI_INSTANCE="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_NETSPEED}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_NETSPEED} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-netspeed.la
#
#
#libcd_netspeed_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-netspeed.c
#	applet-netspeed.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_netspeed_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${netspeeddatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""netspeed.conf""
#	-DMY_APPLET_USER_DATA_DIR=""netspeed""
#	-DMY_APPLET_VERSION=""${VERSION_NETSPEED)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_NETSPEED)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.png""
#	-DCD_APPLET_MULTI_INSTANCE="1"
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_netspeed_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_netspeed_la_LDFLAGS =
#	-avoid-version -module
