 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-clock.la


libcd_clock_la_SOURCES = \
	cd-clock-struct.h\
	cd-clock-init.c\
	cd-clock-init.h\
	cd-clock-draw.c\
	cd-clock-draw.h\
	cd-clock-config.c\
	cd-clock-config.h\
	cd-clock-menu-functions.c\
	cd-clock-menu-functions.h


libcd_clock_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	-DCD_CLOCK_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DCD_CLOCK_README_FILE=\""readme"\"\
	-std=c99\
	-O3
#	-g -ggdb -W -Wall


libcd_clock_la_LIBADD = \
	$(PACKAGE_LIBS) -lm


libcd_clock_la_LDFLAGS =\
	-version-info 1:0:0


moduleclockdir = `pkg-config cairo-dock --variable=pluginsdir`

moduleclock_DATA = \
	.libs/libcd-clock.so

