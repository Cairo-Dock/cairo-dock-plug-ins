#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-desklet-rendering],[1.5.3],[Fabrice Rey (fabounet@glx-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_DESKLET_RENDERING, "$VERSION")
AC_SUBST(PACKAGE_DESKLET_RENDERING, "$PACKAGE")

desklet_renderingdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/desklet-rendering]
AC_SUBST(desklet_renderingdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile])
AC_OUTPUT
