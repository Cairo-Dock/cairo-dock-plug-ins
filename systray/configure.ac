#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-systray],[0.1.8],[GESTES Cedric (ctaf42@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_SYSTRAY=cd-systray
AC_DEFINE_UNQUOTED(GETTEXT_SYSTRAY,"$GETTEXT_SYSTRAY", [Gettext package.])
AC_SUBST(GETTEXT_SYSTRAY)


systraydatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/systray]
AC_SUBST(systraydatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
                data/systray.conf
                po/Makefile.in])
AC_OUTPUT
