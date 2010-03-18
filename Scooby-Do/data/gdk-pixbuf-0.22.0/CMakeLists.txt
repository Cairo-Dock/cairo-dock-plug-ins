configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(gdk-pixbuf)
add_subdirectory(demo)
add_subdirectory(doc)


########### install files ###############

install(FILES  gdk-pixbuf-config DESTINATION )
install(FILES  gdk-pixbuf.m4 DESTINATION /share/aclocal)
install(FILES  gdk_pixbufConf.sh		 	gdk_pixbuf_xlibConf.sh		 	gnomecanvaspixbufConf.sh DESTINATION )



#original Makefile.am contents follow:

#SUBDIRS = gdk-pixbuf demo doc
#
#bin_SCRIPTS = gdk-pixbuf-config
#
#confexecdir = $(libdir)
#confexec_DATA =				\
#	gdk_pixbufConf.sh		\
#	gdk_pixbuf_xlibConf.sh		\
#	gnomecanvaspixbufConf.sh
#
#EXTRA_DIST =				\
#	gdk_pixbufConf.sh.in		\
#	gdk_pixbuf_xlibConf.sh.in	\
#	gnomecanvaspixbufConf.sh.in	\
#	gdk-pixbuf-config.in		\
#	gdk-pixbuf.m4			\
#	gdk-pixbuf.spec			\
#	gdk-pixbuf.spec.in
#
#m4datadir = $(datadir)/aclocal
#m4data_DATA = gdk-pixbuf.m4
#
#gdk_pixbufConf.sh: gdk_pixbufConf.sh.in Makefile
### Use sed and then mv to avoid problems if the user interrupts.
#	sed -e 's?\@GDK_PIXBUF_LIBDIR\@?$(GDK_PIXBUF_LIBDIR)?g' \
#	     -e 's?\@GDK_PIXBUF_INCLUDEDIR\@?$(GDK_PIXBUF_INCLUDEDIR)?g' \
#	     -e 's?\@VERSION\@?$(VERSION)?g' \
#	     -e 's?\@GDK_PIXBUF_LIBS\@?$(GDK_PIXBUF_LIBS)?g' \
#	        < $(srcdir)/gdk_pixbufConf.sh.in > gdk_pixbufConf.tmp \
#	&& mv gdk_pixbufConf.tmp gdk_pixbufConf.sh
#
#gdk_pixbuf_xlibConf.sh: gdk_pixbuf_xlibConf.sh.in Makefile
### Use sed and then mv to avoid problems if the user interrupts.
#	sed -e 's?\@GDK_PIXBUF_XLIB_LIBDIR\@?$(GDK_PIXBUF_XLIB_LIBDIR)?g' \
#	     -e 's?\@GDK_PIXBUF_XLIB_INCLUDEDIR\@?$(GDK_PIXBUF_XLIB_INCLUDEDIR)?g' \
#	     -e 's?\@VERSION\@?$(VERSION)?g' \
#	     -e 's?\@GDK_PIXBUF_XLIB_LIBS\@?$(GDK_PIXBUF_XLIB_LIBS)?g' \
#	        < $(srcdir)/gdk_pixbuf_xlibConf.sh.in > gdk_pixbuf_xlibConf.tmp \
#	&& mv gdk_pixbuf_xlibConf.tmp gdk_pixbuf_xlibConf.sh
#
#gnomecanvaspixbufConf.sh: gnomecanvaspixbufConf.sh.in Makefile
### Use sed and then mv to avoid problems if the user interrupts.
#	sed -e 's?\@GNOMECANVASPIXBUF_LIBDIR\@?$(GNOMECANVASPIXBUF_LIBDIR)?g' \
#	     -e 's?\@GNOMECANVASPIXBUF_INCLUDEDIR\@?$(GNOMECANVASPIXBUF_INCLUDEDIR)?g' \
#	     -e 's?\@VERSION\@?$(VERSION)?g' \
#	     -e 's?\@GNOMECANVASPIXBUF_LIBS\@?$(GNOMECANVASPIXBUF_LIBS)?g' \
#	        < $(srcdir)/gnomecanvaspixbufConf.sh.in > gnomecanvaspixbufConf.tmp \
#	&& mv gnomecanvaspixbufConf.tmp gnomecanvaspixbufConf.sh
