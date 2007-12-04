 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-logout.la


libcd_logout_la_SOURCES = \
	logout-init.c\
	logout-init.h\
	logout-config.c\
	logout-config.h\
	logout-menu-functions.c\
	logout-menu-functions.h


libcd_logout_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libcd_logout_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_logout_la_LDFLAGS =\
	-version-info 1:0:0


modulelogoutdir = `pkg-config cairo-dock --variable=pluginsdir`

modulelogout_DATA = \
	.libs/libcd-logout.so

