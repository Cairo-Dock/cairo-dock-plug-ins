#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-showDesklets],[1.0.4],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_SHOW_DESKLETS, "$VERSION")
AC_SUBST(PACKAGE_SHOW_DESKLETS, "$PACKAGE")

show_deskletsdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/showDesklets]
AC_SUBST(show_deskletsdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/showDesklets.conf])
AC_OUTPUT
