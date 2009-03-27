#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-desklet-rendering],[1.5.1],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_DESKLET_RENDERING=cd-desklet-rendering
AC_DEFINE_UNQUOTED(GETTEXT_DESKLET_RENDERING,"$GETTEXT_DESKLET_RENDERING", [Gettext desklet rendering package.])
AC_SUBST(GETTEXT_DESKLET_RENDERING)

AC_SUBST(VERSION_DESKLET_RENDERING, "$VERSION")
AC_SUBST(PACKAGE_DESKLET_RENDERING, "$PACKAGE")

desklet_renderingdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/desklet-rendering]
AC_SUBST(desklet_renderingdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile])
AC_OUTPUT
