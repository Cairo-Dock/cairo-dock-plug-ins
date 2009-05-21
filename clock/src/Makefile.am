 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-clock.la


libcd_clock_la_SOURCES = \
	applet-struct.h\
	applet-init.c\
	applet-init.h\
	applet-draw.c\
	applet-draw.h\
	applet-digital.c\
	applet-digital.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-theme.c\
	applet-theme.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_clock_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(clockdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.jpg"\"\
	-DMY_APPLET_CONF_FILE=\""clock.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""$(clockuserdirname)"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_CLOCK)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_CLOCK)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-DCD_APPLET_MULTI_INSTANCE=\"1\"\
	-DGL_GLEXT_PROTOTYPES=\""1"\"\
	-std=c99\
	 -Wall\
	-O3
#	-g -ggdb -W -Wall


libcd_clock_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_clock_la_LDFLAGS =\
	-avoid-version -module
