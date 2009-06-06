#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-System-Monitor],[0.0.1],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_SYSTEM_MONITOR=cd-System-Monitor
AC_DEFINE_UNQUOTED(GETTEXT_SYSTEM_MONITOR,"$GETTEXT_SYSTEM_MONITOR", [Gettext package.])
AC_SUBST(GETTEXT_SYSTEM_MONITOR)

AC_SUBST(VERSION_SYSTEM_MONITOR, "$VERSION")
AC_SUBST(PACKAGE_SYSTEM_MONITOR, "$PACKAGE")

system_monitoruserdirname="System-Monitor"
AC_SUBST(system_monitoruserdirname)
system_monitordatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/System-Monitor]
AC_SUBST(system_monitordatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/System-Monitor.conf])
AC_OUTPUT
