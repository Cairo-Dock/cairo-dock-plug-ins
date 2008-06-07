#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-terminal],[1.0.1],[GESTES Cedric (ctaf42@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_TERMINAL=cd-terminal
AC_DEFINE_UNQUOTED(GETTEXT_TERMINAL,"$GETTEXT_TERMINAL", [Gettext package.])
AC_SUBST(GETTEXT_TERMINAL)

AC_SUBST(VERSION_TERMINAL, "$VERSION")
AC_SUBST(PACKAGE_TERMINAL, "$PACKAGE")

terminaldatadir=[`pkg-config cairo-dock --variable=pluginsdir`/terminal]
AC_SUBST(terminaldatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(TERMINAL_PACKAGE, vte,, \
	AC_MSG_ERROR("*** vte not found! You need it to build $PACKAGE_NAME. ***"))

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/readme
		data/terminal.conf
		po/Makefile.in])
AC_OUTPUT
