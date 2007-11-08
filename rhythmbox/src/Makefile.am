 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-rhythmbox.la


libcd_rhythmbox_la_SOURCES = \
	rhythmbox-struct.h\
	rhythmbox-init.c\
	rhythmbox-init.h\
	rhythmbox-draw.c\
	rhythmbox-draw.h\
	rhythmbox-config.c\
	rhythmbox-config.h\
	rhythmbox-dbus.c\
	rhythmbox-dbus.h\
	rhythmbox-menu-functions.c\
	rhythmbox-menu-functions.h


libcd_rhythmbox_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DRHYTHMBOX_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DRHYTHMBOX_README_FILE=\""readme"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libcd_rhythmbox_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_rhythmbox_la_LDFLAGS =\
	-version-info 1:0:0


modulerhythmboxdir = `pkg-config cairo-dock --variable=pluginsdir`

modulerhythmbox_DATA = \
	.libs/libcd-rhythmbox.so

