 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-desklet-rendering.la


libcd_desklet_rendering_la_SOURCES = \
	rendering-init.c\
	rendering-init.h\
	rendering-config.c\
	rendering-config.h\
	rendering-desklet-caroussel.c\
	rendering-desklet-caroussel.h\
	rendering-desklet-tree.c\
	rendering-desklet-tree.h\
	rendering-desklet-simple.h\
	rendering-desklet-simple.c\
	rendering-desklet-controler.h\
	rendering-desklet-controler.c\
	rendering-desklet-mediaplayer.c\
	rendering-desklet-mediaplayer.h\
	rendering-desklet-decorations.c\
	rendering-desklet-decorations.h\
	rendering-desklet-slide.c\
	rendering-desklet-slide.h\
	rendering-struct.h

dock_version = `pkg-config --modversion cairo-dock`

libcd_desklet_rendering_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	$(glitz_cflags) \
	$(include_glitz)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(desklet_renderingdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_DESKLET_RENDERING)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_DESKLET_RENDERING)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_PREVIEW_FILE=\""none"\"\
	-DMY_APPLET_CONF_FILE=\""none"\"\
	-DMY_APPLET_USER_DATA_DIR=\""desklet-rendering"\"\
	-DMY_APPLET_ICON_FILE=\""icon.svg"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-Wno-unused-parameter
#	-g -ggdb -W -Wall


libcd_desklet_rendering_la_LIBADD = \
	$(glitz_libs) \
	$(PACKAGE_LIBS) -lm


libcd_desklet_rendering_la_LDFLAGS = \
	-avoid-version -module
