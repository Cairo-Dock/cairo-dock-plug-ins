#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-show_mouse],[1.0.2],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_SHOW_MOUSE=cd-show_mouse
AC_DEFINE_UNQUOTED(GETTEXT_SHOW_MOUSE,"$GETTEXT_SHOW_MOUSE", [Gettext package.])
AC_SUBST(GETTEXT_SHOW_MOUSE)

AC_SUBST(VERSION_SHOW_MOUSE, "$VERSION")
AC_SUBST(PACKAGE_SHOW_MOUSE, "$PACKAGE")


show_mousedatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/show_mouse]
AC_SUBST(show_mousedatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/show_mouse.conf])
AC_OUTPUT
