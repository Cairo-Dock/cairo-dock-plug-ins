#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Scooby-Do],[0.0.1],[Fabrice Rey (Fabounet) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_SCOOBY_DO=cd-Scooby-Do
AC_DEFINE_UNQUOTED(GETTEXT_SCOOBY_DO,"$GETTEXT_SCOOBY_DO", [Gettext package.])
AC_SUBST(GETTEXT_SCOOBY_DO)

AC_SUBST(VERSION_SCOOBY_DO, "$VERSION")
AC_SUBST(PACKAGE_SCOOBY_DO, "$PACKAGE")

scooby_douserdirname="Toons"
AC_SUBST(scooby_douserdirname)
scooby_dodatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Scooby-Do]
AC_SUBST(scooby_dodatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/Scooby-Do.conf])
AC_OUTPUT
