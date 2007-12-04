 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-dustbin.la


libcd_dustbin_la_SOURCES = \
	dustbin-struct.h\
	dustbin-init.c\
	dustbin-init.h\
	dustbin-draw.c\
	dustbin-draw.h\
	dustbin-config.c\
	dustbin-config.h\
	dustbin-menu-functions.c\
	dustbin-menu-functions.h


libcd_dustbin_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libcd_dustbin_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_dustbin_la_LDFLAGS =\
	-version-info 1:0:0


moduledustbindir = `pkg-config cairo-dock --variable=pluginsdir`

moduledustbin_DATA = \
	.libs/libcd-dustbin.so
