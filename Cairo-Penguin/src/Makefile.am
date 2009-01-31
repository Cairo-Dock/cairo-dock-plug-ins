 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-Cairo-Penguin.la


libcd_Cairo_Penguin_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-load-icons.c\
	applet-load-icons.h\
	applet-animation.c\
	applet-animation.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_Cairo_Penguin_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(cairo_penguindatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""Cairo-Penguin.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""$(cairo_penguinuserdirname)"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_CAIRO_PENGUIN)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_CAIRO_PENGUIN)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-DCD_APPLET_MULTI_INSTANCE=\"1\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_Cairo_Penguin_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_Cairo_Penguin_la_LDFLAGS =\
	-avoid-version -module
