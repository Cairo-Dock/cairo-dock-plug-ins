#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-GMenu],[1.0.0],[Fabounet (Fabrice Rey) (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_GMENU=cd-GMenu
AC_DEFINE_UNQUOTED(GETTEXT_GMENU,"$GETTEXT_GMENU", [Gettext package.])
AC_SUBST(GETTEXT_GMENU)

AC_SUBST(VERSION_GMENU, "$VERSION")
AC_SUBST(PACKAGE_GMENU, "$PACKAGE")

PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(GMENU_PACKAGE, libgnome-menu)

AC_ARG_ENABLE(gio,
	[  --enable-gio  :  Use gio],
	[case "${enableval}" in
		yes) usegio=true ;;
		no)  usegio=false ;;
		*) echo "bad value ${enableval} for --enable-gio" ;;
	esac],[usegio=false])

if test "$usegio" = "true" ; then
	PKG_CHECK_MODULES(GMENU_GIO, gio-2.0, [gio_detected=true], [ gio_detected=false; AC_MSG_WARN(Could not find gio; GMenu won't be built with. Depending on libgnome-menu, it may not be a problem.) ])
	AC_SUBST(GMENU_GIO_CFLAGS)
	AC_SUBST(GMENU_GIO_LIBS)
	
	if test ! "$gio_detected" = "false" ; then
		AC_DEFINE(HAVE_GIO,"1",[Defined if we can use gio.])
	fi
fi

AM_CONDITIONAL( [WITH_GIO_IN_GMENU], [test "$gio_detected" = "true"] )


gmenudatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/GMenu]
AC_SUBST(gmenudatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/GMenu.conf])
AC_OUTPUT
