 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-logout.la


libcd_logout_la_SOURCES = \
	applet-init.c\
	applet-init.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h


libcd_logout_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_PREVIEW_FILE=\""preview.png"\"\
	-DMY_APPLET_CONF_FILE=\""logout.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""logout"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""cd-logout"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-g -ggdb -W -Wall


libcd_logout_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_logout_la_LDFLAGS =\
	-version-info 1:0:0


modulelogoutdir = `pkg-config cairo-dock --variable=pluginsdir`

modulelogout_DATA = \
	.libs/libcd-logout.so

