
########### next target ###############

SET(cd-Clipper_LIB_SRCS
	applet-init.c 		applet-init.h
	applet-config.c 		applet-config.h
	applet-notifications.c 	applet-notifications.h
	applet-clipboard.c 	applet-clipboard.h
	applet-struct.h
)

add_library(${PACKAGE_CLIPPER} SHARED ${cd-Clipper_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${Clipperdatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="Clipper.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="Clipper")
add_definitions (-DMY_APPLET_VERSION="${VERSION_CLIPPER}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_CLIPPER}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_CLIPPER}
	${PACKAGE_LIBRARIES})

install(TARGETS ${PACKAGE_CLIPPER} DESTINATION ${pluginsdir})


########### install files ###############




#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-Clipper.la
#
#
#libcd_Clipper_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-clipboard.c
#	applet-clipboard.h
#	applet-struct.h
#
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_Clipper_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${Clipperdatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""Clipper.conf""
#	-DMY_APPLET_USER_DATA_DIR=""Clipper""
#	-DMY_APPLET_VERSION=""${VERSION_CLIPPER)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_CLIPPER)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_ICON_FILE=""icon.svg""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_Clipper_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_Clipper_la_LDFLAGS =
#	-avoid-version -module
