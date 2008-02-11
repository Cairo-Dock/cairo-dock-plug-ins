 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-gnome-integration.la


libcd_gnome_integration_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-gnome-vfs.c\
	applet-gnome-vfs.h\
	applet-utils.c\
	applet-utils.h


dock_version = `pkg-config --modversion cairo-dock`
gnome_vfs_version = `pkg-config --modversion gnome-vfs-2.0`

libcd_gnome_integration_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\"none""\"\
	-DMY_APPLET_CONF_FILE=\""none"\"\
	-DMY_APPLET_USER_DATA_DIR=\""gnome-integration"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""cd-gnome-integration"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DGNOME_VFS_VERSION=\""$(gnome_vfs_version)"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_gnome_integration_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_gnome_integration_la_LDFLAGS =\
	-version-info 1:0:0


moduledir = `pkg-config cairo-dock --variable=pluginsdir`

module_DATA = \
	.libs/libcd-gnome-integration.so

