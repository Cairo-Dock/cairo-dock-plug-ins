 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-gnome-integration-old.la


libcd_gnome_integration_old_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-gnome-vfs.c\
	applet-gnome-vfs.h\
	applet-utils.c\
	applet-utils.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_gnome_integration_old_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	$(OLD_GNOME_INTEGRATION_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(gnome_integration_olddatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\"none""\"\
	-DMY_APPLET_CONF_FILE=\""none"\"\
	-DMY_APPLET_USER_DATA_DIR=\""gnome-integration-old"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_GNOME_INTEGRATION_OLD)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""cd-gnome-integration-old"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.svg"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_gnome_integration_old_la_LIBADD = \
	$(PACKAGE_LIBS)\
	$(OLD_GNOME_INTEGRATION_LIBS)\
	-lm


libcd_gnome_integration_old_la_LDFLAGS =\
	-avoid-version -module
