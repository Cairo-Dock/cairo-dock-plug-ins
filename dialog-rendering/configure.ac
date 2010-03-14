#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-dialog-rendering],[0.4.2],[Fabrice Rey (Fabounet) (fabounet@glx-dock.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_DIALOG_RENDERING, "$VERSION")
AC_SUBST(PACKAGE_DIALOG_RENDERING, "$PACKAGE")

dialog_renderingdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/dialog-rendering]
AC_SUBST(dialog_renderingdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/dialog-rendering.conf])
AC_OUTPUT
