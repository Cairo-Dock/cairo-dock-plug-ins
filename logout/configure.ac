#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-logout],[1.2.3],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_LOGOUT=cd-logout
AC_DEFINE_UNQUOTED(GETTEXT_LOGOUT,"$GETTEXT_LOGOUT", [Gettext package.])
AC_SUBST(GETTEXT_LOGOUT)

AC_SUBST(VERSION_LOGOUT, "$VERSION")
AC_SUBST(PACKAGE_LOGOUT, "$PACKAGE")


logoutdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/logout]
AC_SUBST(logoutdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/logout.conf])
AC_OUTPUT
