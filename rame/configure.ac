#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-rame],[1.0.0-beta1],[parAdOxxx_ZeRo (changfu@hollowproject.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_RAME=cd-rame
AC_DEFINE_UNQUOTED(GETTEXT_RAME,"$GETTEXT_RAME", [Gettext package.])
AC_SUBST(GETTEXT_RAME)

AC_SUBST(VERSION_RAME, "$VERSION")
AC_SUBST(PACKAGE_RAME, "$PACKAGE")

ramedatadir=[`pkg-config cairo-dock --variable=pluginsdir`/rame]
AC_SUBST(ramedatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/rame.conf])
AC_OUTPUT
