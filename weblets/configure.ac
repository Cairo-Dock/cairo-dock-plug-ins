#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-weblets],[0.0.7],[Christophe Chapuis (chris.chapuis@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


webletsdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/weblets]
AC_SUBST(webletsdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

PKG_CHECK_MODULES(WEBKIT,webkit-1.0, [webkit_detected=true], [ webkit_detected=false; AC_MSG_ERROR(Could not find webkit-1.0; Weblets plugin requires webkit to be installed.) ])
AC_SUBST(WEBKIT_CFLAGS)
AC_SUBST(WEBKIT_LIBS)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/weblets.conf])
AC_OUTPUT
