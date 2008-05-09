 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-rendering.la

if COMPIL_WITH_GLITZ
include_glitz = -DHAVE_GLITZ=TRUE
glitz_cflags = $(GLITZ_CFLAGS)
glitz_libs = $(GLITZ_LIBS)
endif


libcd_rendering_la_SOURCES = \
	rendering-init.c\
	rendering-init.h\
	rendering-config.c\
	rendering-config.h\
	rendering-caroussel.c\
	rendering-caroussel.h\
	rendering-parabole.c\
	rendering-parabole.h\
	rendering-3D-plane.c\
	rendering-3D-plane.h\
	rendering-rainbow.c\
	rendering-rainbow.h\
	rendering-diapo.c\
	rendering-diapo.h\
	rendering-desklet-caroussel.c\
	rendering-desklet-caroussel.h\
	rendering-desklet-tree.c\
	rendering-desklet-tree.h\
	rendering-desklet-simple.h\
	rendering-desklet-simple.c\
	rendering-desklet-controler.h\
	rendering-desklet-controler.c

dock_version = `pkg-config --modversion cairo-dock`

libcd_rendering_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	$(glitz_cflags) \
	$(include_glitz)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_PACKAGE)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_PREVIEW_FILE=\""none"\"\
	-DMY_APPLET_CONF_FILE=\""rendering.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""rendering"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-Wno-unused-parameter
#	-g -ggdb -W -Wall


libcd_rendering_la_LIBADD = \
	$(glitz_libs) \
	$(PACKAGE_LIBS) -lm


libcd_rendering_la_LDFLAGS =\
	-version-info 1:0:0


moduledir = `pkg-config cairo-dock --variable=pluginsdir`

module_DATA = \
	.libs/libcd-rendering.so

