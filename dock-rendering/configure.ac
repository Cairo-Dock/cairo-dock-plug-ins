#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-rendering],[1.5.2],[Fabrice Rey (fabounet@users.berlios.de) & paradoxxx_zero])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_RENDERING=cd-rendering
AC_DEFINE_UNQUOTED(GETTEXT_RENDERING,"$GETTEXT_RENDERING", [Gettext rendering package.])
AC_SUBST(GETTEXT_RENDERING)

AC_SUBST(VERSION_RENDERING, "$VERSION")
AC_SUBST(PACKAGE_RENDERING, "$PACKAGE")

renderingdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/rendering]
AC_SUBST(renderingdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdir`/rendering]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/rendering.conf])
AC_OUTPUT
