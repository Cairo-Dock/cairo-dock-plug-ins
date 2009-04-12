 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-Scooby-Do.la


libcd_Scooby_Do_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h\
	applet-icon-finder.c\
	applet-icon-finder.h\
	applet-command-finder.c\
	applet-command-finder.h\
	applet-draw.c\
	applet-draw.h\
	applet-struct.h


dock_version = `pkg-config --modversion cairo-dock`

libcd_Scooby_Do_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""Scooby-Do.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""Scooby-Do"\"\
	-DMY_APPLET_VERSION=\""$(VERSION_SCOOBY_DO)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""$(GETTEXT_SCOOBY_DO)"\"\
	-DMY_APPLET_DOCK_VERSION=\""$(dock_version)"\"\
	-DMY_APPLET_ICON_FILE=\""icon.png"\"\
	-DGL_GLEXT_PROTOTYPES=\""1"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_Scooby_Do_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_Scooby_Do_la_LDFLAGS =\
	-avoid-version -module
