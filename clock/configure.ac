#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-clock],[2.0.9],[Fabrice Rey (fabounet@glx-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


AC_SUBST(VERSION_CLOCK, "$VERSION")
AC_SUBST(PACKAGE_CLOCK, "$PACKAGE")

clockuserdirname="clock"
AC_SUBST(clockuserdirname)
clockdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/clock]
AC_SUBST(clockdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/clock.conf
		data/digital/Makefile
		data/digital/default/Makefile
		data/themes/Makefile
		data/themes/default/Makefile
		])
AC_OUTPUT
