#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Clipper],[1.0.4],[Fabrice Rey (Fabounet) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


AC_SUBST(VERSION_CLIPPER, "$VERSION")
AC_SUBST(PACKAGE_CLIPPER, "$PACKAGE")

Clipperdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Clipper]
AC_SUBST(Clipperdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/Clipper.conf])
AC_OUTPUT
