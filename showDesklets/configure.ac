#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-showDesklets],[1.0.4],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_SHOW_DESKLETS=cd-showDesklets
AC_DEFINE_UNQUOTED(GETTEXT_SHOW_DESKLETS,"$GETTEXT_SHOW_DESKLETS", [Gettext package.])
AC_SUBST(GETTEXT_SHOW_DESKLETS)

AC_SUBST(VERSION_SHOW_DESKLETS, "$VERSION")
AC_SUBST(PACKAGE_SHOW_DESKLETS, "$PACKAGE")

show_deskletsdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/showDesklets]
AC_SUBST(show_deskletsdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/showDesklets.conf])
AC_OUTPUT
