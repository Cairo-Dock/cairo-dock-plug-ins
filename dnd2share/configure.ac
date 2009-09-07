#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-dnd2share],[1.0.0-beta2],[Yann Dulieu (nochka85@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_DND2SHARE, "$VERSION")
AC_SUBST(PACKAGE_DND2SHARE, "$PACKAGE")

dnd2sharedatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/dnd2share]
AC_SUBST(dnd2sharedatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/dnd2share.conf])
AC_OUTPUT
