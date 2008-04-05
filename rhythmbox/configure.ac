#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-rhythmbox],[1.1.2],[Adrien Pilleboue (adrien.pilleboue@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=cd-rhythmbox
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)


pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdir`/rhythmbox]
AC_SUBST(pkgdatadir)


PKG_CHECK_MODULES(PACKAGE, "cairo-dock dbus-glib-1 dbus-1")

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/rhythmbox.conf
		data/readme])
AC_OUTPUT
