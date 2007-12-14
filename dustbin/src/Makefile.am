 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-dustbin.la


libcd_dustbin_la_SOURCES = \
	applet-struct.h\
	applet-init.c\
	applet-init.h\
	applet-draw.c\
	applet-draw.h\
	applet-config.c\
	applet-config.h\
	applet-notifications.c\
	applet-notifications.h


libcd_dustbin_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_CONF_FILE=\""dustbin.conf"\"\
	-DMY_APPLET_USER_DATA_DIR=\""dustbin"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libcd_dustbin_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_dustbin_la_LDFLAGS =\
	-version-info 1:0:0


moduledir = `pkg-config cairo-dock --variable=pluginsdir`
module_DATA = \
	.libs/libcd-dustbin.so
