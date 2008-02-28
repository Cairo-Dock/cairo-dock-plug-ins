 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-xfce-integration.la


libcd_xfce_integration_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-thunar-vfs.c\
	applet-thunar-vfs.h\
	applet-utils.c\
	applet-utils.h


dock_version = `pkg-config --modversion cairo-dock`
thunar_vfs_version = `pkg-config --modversion thunar-vfs-1`

libcd_xfce_integration_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\"none""\"\
	-DMY_APPLET_CONF_FILE=\""none"\"\
	-DMY_APPLET_USER_DATA_DIR=\""xfce-integration"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""cd-xfce-integration"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DTHUNAR_VFS_VERSION=\""$(thunar_vfs_version)"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_xfce_integration_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_xfce_integration_la_LDFLAGS =\
	-version-info 1:0:0


moduledir = `pkg-config cairo-dock --variable=pluginsdir`

module_DATA = \
	.libs/libcd-xfce-integration.so

