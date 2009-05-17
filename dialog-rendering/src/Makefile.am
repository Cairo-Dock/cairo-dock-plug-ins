 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-dialog-rendering.la


libcd_dialog_rendering_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-renderer-text.c\
	applet-renderer-text.h\
	applet-decorator-comics.c\
	applet-decorator-comics.h\
	applet-decorator-modern.c\
	applet-decorator-modern.h\
	applet-decorator-3Dplane.c\
	applet-decorator-3Dplane.h\
	applet-decorator-tooltip.c\
	applet-decorator-tooltip.h\
	applet-decorator-curly.c\
	applet-decorator-curly.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_dialog_rendering_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(dialog_renderingdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""none"\"\
	-DMY_APPLET_CONF_FILE=\""dialog-rendering.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""dialog-rendering"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_DIALOG_RENDERING)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_DIALOG_RENDERING)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.svg"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_dialog_rendering_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_dialog_rendering_la_LDFLAGS =\
	-avoid-version -module
