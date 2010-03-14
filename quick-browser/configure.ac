#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-quick-browser],[1.0.5],[Fabounet (Fabrice Rey) (fabounet@glx-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_QUICK_BROWSER, "$VERSION")
AC_SUBST(PACKAGE_QUICK_BROWSER, "$PACKAGE")

quick_browserdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/quick-browser]
AC_SUBST(quick_browserdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/quick-browser.conf])
AC_OUTPUT
