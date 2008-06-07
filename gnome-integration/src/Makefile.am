 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-gnome-integration.la


libcd_gnome_integration_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-gvfs.c\
	applet-gvfs.h\
	applet-utils.c\
	applet-utils.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_gnome_integration_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	$(GNOME_INTEGRATION_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(gnome_integrationdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\"none""\"\
	-DMY_APPLET_CONF_FILE=\""none"\"\
	-DMY_APPLET_USER_DATA_DIR=\""gnome-integration"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_GNOME_INTEGRATION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""cd-gnome-integration"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_gnome_integration_la_LIBADD = \
	$(PACKAGE_LIBS)\
	$(GNOME_INTEGRATION_LIBS)\
	-lm


libcd_gnome_integration_la_LDFLAGS =\
	-avoid-version
