#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-showDesktop],[1.0.2],[Romain Perol ()])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_SHOW_DESKTOP=cd-showDesktop
AC_DEFINE_UNQUOTED(GETTEXT_SHOW_DESKTOP,"$GETTEXT_SHOW_DESKTOP", [Gettext package.])
AC_SUBST(GETTEXT_SHOW_DESKTOP)

AC_SUBST(VERSION_SHOW_DESKTOP, "$VERSION")
AC_SUBST(PACKAGE_SHOW_DESKTOP, "$PACKAGE")

show_desktopdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/showDesktop]
AC_SUBST(show_desktopdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/showDesktop.conf])
AC_OUTPUT
