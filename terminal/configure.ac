#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-terminal],[1.0.7],[GESTES Cedric (ctaf42@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


AC_SUBST(VERSION_TERMINAL, "$VERSION")
AC_SUBST(PACKAGE_TERMINAL, "$PACKAGE")

terminaldatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/terminal]
AC_SUBST(terminaldatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(TERMINAL_PACKAGE, vte,, \
	AC_MSG_ERROR("*** vte not found! You need it to build $PACKAGE_NAME. ***"))

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/terminal.conf])
AC_OUTPUT
