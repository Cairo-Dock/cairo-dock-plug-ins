#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Toons],[1.0.3],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_TOONS, "$VERSION")
AC_SUBST(PACKAGE_TOONS, "$PACKAGE")

toonsuserdirname="Toons"
AC_SUBST(toonsuserdirname)
toonsdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Toons]
AC_SUBST(toonsdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/Toons.conf
		data/themes/Makefile
		data/themes/Classic/Makefile
		])
AC_OUTPUT
