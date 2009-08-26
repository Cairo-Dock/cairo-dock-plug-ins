#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd_gnome-integration-old],[1.0.2],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


gnome_integration_olddatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/gnome-integration-old]
AC_SUBST(gnome_integration_olddatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

AC_SUBST(VERSION_GNOME_INTEGRATION_OLD, "$VERSION")
AC_SUBST(PACKAGE_GNOME_INTEGRATION_OLD, "$PACKAGE")


PKG_CHECK_MODULES(PACKAGE, cairo-dock gnome-vfs-2.0 libgnomeui-2.0)
PKG_CHECK_MODULES(OLD_GNOME_INTEGRATION,gnome-vfs-2.0 libgnomeui-2.0)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		])
AC_OUTPUT
