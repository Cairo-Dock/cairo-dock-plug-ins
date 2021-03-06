
########### sources ###############

SET(cd-illusion_LIB_SRCS
	applet-init.c
	applet-init.h
	applet-config.c
	applet-config.h
	applet-struct.h
	applet-notifications.c
	applet-notifications.h
	applet-evaporate.c
	applet-evaporate.h
	applet-fade-out.c
	applet-fade-out.h
	applet-explode.c
	applet-explode.h
	applet-break.c
	applet-break.h
	applet-black-hole.c
	applet-black-hole.h
	applet-lightning.c
	applet-lightning.h
	evaporate-tex.h
)

add_library(${PACKAGE_ILLUSION} SHARED ${cd-illusion_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${illusiondatadir}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="preview.jpg")
add_definitions (-DMY_APPLET_CONF_FILE="illusion.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="illusion")
add_definitions (-DMY_APPLET_VERSION="${VERSION_ILLUSION}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_ILLUSION}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_ILLUSION}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_ILLUSION} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-illusion.la
#
#
#libcd_illusion_la_SOURCES = 
#	applet-init.c
#	applet-init.h
#	applet-config.c
#	applet-config.h
#	applet-struct.h
#	applet-notifications.c
#	applet-notifications.h
#	applet-evaporate.c
#	applet-evaporate.h
#	applet-fade-out.c
#	applet-fade-out.h
#	applet-explode.c
#	applet-explode.h
#	applet-break.c
#	applet-break.h
#	applet-black-hole.c
#	applet-black-hole.h
#	applet-lightning.c
#	applet-lightning.h
#	evaporate-tex.h
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_illusion_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	-DMY_APPLET_SHARE_DATA_DIR=""${illusiondatadir)""
#	-DMY_APPLET_PREVIEW_FILE=""preview.jpg""
#	-DMY_APPLET_CONF_FILE=""illusion.conf""
#	-DMY_APPLET_USER_DATA_DIR=""illusion""
#	-DMY_APPLET_VERSION=""${VERSION_ILLUSION)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_ILLUSION)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DGL_GLEXT_PROTOTYPES=""1""
#	-DMY_APPLET_ICON_FILE=""icon.svg""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-g -ggdb -W -Wall
#
#
#libcd_illusion_la_LIBADD = 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_illusion_la_LDFLAGS =
#	-avoid-version -module
