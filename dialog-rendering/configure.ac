#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-dialog-rendering],[0.4.2],[Fabrice Rey (Fabounet) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_DIALOG_RENDERING=cd-dialog-rendering
AC_DEFINE_UNQUOTED(GETTEXT_DIALOG_RENDERING,"$GETTEXT_DIALOG_RENDERING", [Gettext package.])
AC_SUBST(GETTEXT_DIALOG_RENDERING)

AC_SUBST(VERSION_DIALOG_RENDERING, "$VERSION")
AC_SUBST(PACKAGE_DIALOG_RENDERING, "$PACKAGE")

dialog_renderingdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/dialog-rendering]
AC_SUBST(dialog_renderingdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/dialog-rendering.conf])
AC_OUTPUT
