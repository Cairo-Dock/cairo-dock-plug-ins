#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-icon-effect],[1.1.3],[Fabrice Rey (Fabounet) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_ICON_EFFECTS, "$VERSION")
AC_SUBST(PACKAGE_ICON_EFFECTS, "$PACKAGE")

icon_effectsdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/icon-effect]
AC_SUBST(icon_effectsdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
	src/Makefile
	data/Makefile
	data/icon-effect.conf])
AC_OUTPUT
