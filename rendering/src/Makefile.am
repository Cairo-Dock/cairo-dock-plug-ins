 #dnl Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = \
	libcd-rendering.la

if COMPIL_WITH_GLITZ
include_glitz = -DHAVE_GLITZ=TRUE
glitz_cflags = $(GLITZ_CFLAGS)
glitz_libs = $(GLITZ_LIBS)
endif


libcd_rendering_la_SOURCES = \
	rendering-init.c\
	rendering-init.h\
	rendering-config.c\
	rendering-config.h\
	rendering-caroussel.c\
	rendering-caroussel.h\
	rendering-parabole.c\
	rendering-parabole.h\
	rendering-3D-plane.c\
	rendering-3D-plane.h


libcd_rendering_la_CFLAGS = \
	-I$(top_srcdir)/src\
	$(PACKAGE_CFLAGS)\
	$(glitz_cflags) \
	$(include_glitz)\
	-DMY_APPLET_SHARE_DATA_DIR=\""$(pkgdatadir)"\"\
	-DMY_APPLET_README_FILE=\""readme"\"\
	-DMY_APPLET_VERSION=\""$(VERSION)"\"\
	-DMY_APPLET_GETTEXT_DOMAIN=\""cd-rendering"\"\
	-std=c99\
	-Werror-implicit-function-declaration\
	-O3
#	-Wno-unused-parameter
#	-g -ggdb -W -Wall


libcd_rendering_la_LIBADD = \
	$(glitz_libs) \
	$(PACKAGE_LIBS) -lm


libcd_rendering_la_LDFLAGS =\
	-version-info 1:0:0


modulerenderingdir = `pkg-config cairo-dock --variable=pluginsdir`

modulerendering_DATA = \
	.libs/libcd-rendering.so

