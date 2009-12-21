#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-showDesktop],[1.2.1],[Romain Perol ()])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(VERSION_SHOW_DESKTOP, "$VERSION")
AC_SUBST(PACKAGE_SHOW_DESKTOP, "$PACKAGE")

show_desktopdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/showDesktop]
AC_SUBST(show_desktopdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(SHOW_DESKTOP_PACKAGE, xrandr)

AC_ARG_ENABLE(xrandr,
	[  --enable-xrandr  :  Use xrandr],
	[case "${enableval}" in
		yes) usexrandr=true ;;
		no)  usexrandr=false ;;
		*) echo "bad value ${enableval} for --enable-xrandr" ;;
	esac],[usexrandr=false])

if test "$usexrandr" = "true" ; then
	PKG_CHECK_MODULES(SHOW_DESKTOP_XRANDR, xrandr, [xrandr_detected=true], [ xrandr_detected=false; AC_MSG_WARN(Could not find xrandr; ShowDesktop won't be built with screen resolution abilities.) ])
	AC_SUBST(SHOW_DESKTOP_XRANDR_CFLAGS)
	AC_SUBST(SHOW_DESKTOP_XRANDR_LIBS)
	
	if test ! "$xrandr_detected" = "false" ; then
		AC_DEFINE(HAVE_XRANDR,"1",[Defined if we can use xrandr.])
	fi
fi

AM_CONDITIONAL( [WITH_XRANDR_IN_SHOW_DESKTOP], [test "$xrandr_detected" = "true"] )

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile
		data/showDesktop.conf])
AC_OUTPUT
