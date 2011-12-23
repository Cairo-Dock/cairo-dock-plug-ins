
########### sources ###############

SET(cd-rendering_LIB_SRCS
	rendering-init.c 		rendering-init.h
	rendering-config.c 		rendering-config.h
	#rendering-caroussel.c 		rendering-caroussel.h
	rendering-parabole.c 		rendering-parabole.h
	rendering-3D-plane.c 		rendering-3D-plane.h
	rendering-rainbow.c 		rendering-rainbow.h
	rendering-diapo-simple.c 	rendering-diapo-simple.h
	rendering-curve.c 		rendering-curve.h
	rendering-panel.c 		rendering-panel.h
	rendering-commons.c		rendering-commons.h
	rendering-struct.h
)

add_library(${PACKAGE_RENDERING} SHARED ${cd-rendering_LIB_SRCS})

########### compil ###############
add_definitions (-DMY_APPLET_SHARE_DATA_DIR="${renderingdatadir}")
add_definitions (-DMY_APPLET_VERSION="${VERSION_RENDERING}")
add_definitions (-DMY_APPLET_GETTEXT_DOMAIN="${GETTEXT_RENDERING}")
add_definitions (-DMY_APPLET_DOCK_VERSION="${dock_version}")
add_definitions (-DMY_APPLET_PREVIEW_FILE="none")
add_definitions (-DMY_APPLET_CONF_FILE="rendering.conf")
add_definitions (-DMY_APPLET_USER_DATA_DIR="rendering")
add_definitions (-DMY_APPLET_ICON_FILE="icon.svg")
add_definitions (-DGL_GLEXT_PROTOTYPES="1")

include_directories (
	${PACKAGE_INCLUDE_DIRS})

link_directories (
	${PACKAGE_LIBRARY_DIRS})

target_link_libraries (${PACKAGE_RENDERING}
	${PACKAGE_LIBRARIES})

########### install files ###############

install(TARGETS ${PACKAGE_RENDERING} DESTINATION ${pluginsdir})


#original Makefile.am contents follow:

# #dnl Process this file with automake to produce Makefile.in
#
#lib_LTLIBRARIES = 
#	libcd-rendering.la
#
#
#libcd_rendering_la_SOURCES = 
#	rendering-init.c
#	rendering-init.h
#	rendering-config.c
#	rendering-config.h
#	rendering-caroussel.c
#	rendering-caroussel.h
#	rendering-parabole.c
#	rendering-parabole.h
#	rendering-3D-plane.c
#	rendering-3D-plane.h
#	rendering-rainbow.c
#	rendering-rainbow.h
#	rendering-diapo-simple.c
#	rendering-diapo-simple.h
#	rendering-curve.c
#	rendering-curve.h
#	rendering-commons.c
#	rendering-commons.h
#	rendering-struct.h
#
#dock_version = `pkg-config --modversion cairo-dock`
#
#libcd_rendering_la_CFLAGS = 
#	-I$(top_srcdir)/src
#	$(PACKAGE_CFLAGS)
#	$(glitz_cflags) 
#	$(include_glitz)
#	-DMY_APPLET_SHARE_DATA_DIR=""${renderingdatadir)""
#	-DMY_APPLET_VERSION=""${VERSION_RENDERING)""
#	-DMY_APPLET_GETTEXT_DOMAIN=""${GETTEXT_RENDERING)""
#	-DMY_APPLET_DOCK_VERSION=""${dock_version)""
#	-DMY_APPLET_PREVIEW_FILE=""none""
#	-DMY_APPLET_CONF_FILE=""rendering.conf""
#	-DMY_APPLET_USER_DATA_DIR=""rendering""
#	-DMY_APPLET_ICON_FILE=""icon.svg""
#	-DGL_GLEXT_PROTOTYPES=""1""
#	-std=c99
#	-Werror-implicit-function-declaration
##	-Wno-unused-parameter
##	-g -ggdb -W -Wall
#
#
#libcd_rendering_la_LIBADD = 
#	$(glitz_libs) 
#	$(PACKAGE_LIBS) -lm
#
#
#libcd_rendering_la_LDFLAGS = 
#	-avoid-version -module
