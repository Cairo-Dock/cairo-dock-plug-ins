#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-icon-effect],[1.1.3],[Fabrice Rey (Fabounet) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_ICON_EFFECTS=cd-icon-effect
AC_DEFINE_UNQUOTED(GETTEXT_ICON_EFFECTS,"$GETTEXT_ICON_EFFECTS", [Gettext package.])
AC_SUBST(GETTEXT_ICON_EFFECTS)

AC_SUBST(VERSION_ICON_EFFECTS, "$VERSION")
AC_SUBST(PACKAGE_ICON_EFFECTS, "$PACKAGE")


icon_effectsdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/icon-effect]
AC_SUBST(icon_effectsdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
	src/Makefile
	po/Makefile.in
	data/Makefile
	data/icon-effect.conf])
AC_OUTPUT
