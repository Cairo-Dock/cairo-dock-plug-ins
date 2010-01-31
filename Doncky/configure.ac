#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Doncky],[0.0.1],[Yann Dulieu (Nochka85) (nochka85@cairo-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


AC_SUBST(VERSION_DONCKY, "$VERSION")
AC_SUBST(PACKAGE_DONCKY, "$PACKAGE")

donckyuserdirname="Doncky"
AC_SUBST(donckyuserdirname)
donckydatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Doncky]
AC_SUBST(donckydatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/Doncky.conf])
AC_OUTPUT
