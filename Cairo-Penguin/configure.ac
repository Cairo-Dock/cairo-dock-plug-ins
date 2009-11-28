#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Cairo-Penguin],[1.1.5],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_CAIRO_PENGUIN, "$VERSION")
AC_SUBST(PACKAGE_CAIRO_PENGUIN, "$PACKAGE")

cairo_penguinuserdirname="Cairo-Penguin"
AC_SUBST(cairo_penguinuserdirname)
cairo_penguindatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Cairo-Penguin]
AC_SUBST(cairo_penguindatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/Cairo-Penguin.conf
		data/themes/Makefile
		data/themes/Classic/Makefile
		])
AC_OUTPUT
