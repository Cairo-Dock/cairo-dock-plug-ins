 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-rhythmbox.la


libcd_rhythmbox_la_SOURCES = \
	rhythmbox-init.c\
	rhythmbox-init.h\
	rhythmbox-draw.c\
	rhythmbox-draw.h\
	rhythmbox-config.c\
	rhythmbox-config.h\
	rhythmbox-dbus.c\
	rhythmbox-dbus.h\
	rhythmbox-menu-functions.c\
	rhythmbox-menu-functions.h\
	rhythmbox-struct.h\
	3dcover-draw.c\
	3dcover-draw.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_rhythmbox_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(rhythmboxdatadir)"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""rhythmbox.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""rhythmbox"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_RHYTHMBOX)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_RHYTHMBOX)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-DGL_GLEXT_PROTOTYPES=\""1"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_rhythmbox_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_rhythmbox_la_LDFLAGS =\
	-avoid-version -module
