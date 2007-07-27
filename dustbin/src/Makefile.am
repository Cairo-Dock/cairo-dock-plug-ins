 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libdustbin.la


libdustbin_la_SOURCES = \
	dustbin-sdustbint.h\
	dustbin-init.c\
	dustbin-init.h\
	dustbin-draw.c\
	dustbin-draw.h\
	dustbin-config.c\
	dustbin-config.h\
	dustbin-menu-functions.c\
	dustbin-menu-functions.h


libdustbin_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DCD_DUSTBIN_SHARE_DATA_DIR=\""$(datadir)/dustbin"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libdustbin_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libdustbin_la_LDFLAGS =\
	-version-info 1:0:0


moduledustbindir = `pkg-config cairo-dock --variable=pluginsdir`

moduledustbin_DATA = \
	.libs/libdustbin.so

