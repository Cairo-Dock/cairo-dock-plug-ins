#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-Scooby-Do],[0.2.0],[Fabrice Rey (Fabounet) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_SCOOBY_DO, "$VERSION")
AC_SUBST(PACKAGE_SCOOBY_DO, "$PACKAGE")

scooby_douserdirname="Scooby-Do"
AC_SUBST(scooby_douserdirname)
scooby_dodatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/Scooby-Do]
AC_SUBST(scooby_dodatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/Scooby-Do.conf])
AC_OUTPUT
