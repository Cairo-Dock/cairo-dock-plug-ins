#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-weblets],[0.0.2],[Christophe Chapuis (chris.chapuis@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=cd-weblets
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)


pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/weblets]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_ARG_ENABLE(webkit,
	[  --enable-webkit  :  Use WebKit backend instead of Gecko],
	[case "${enableval}" in
		yes) usewebkit=true ;;
		no)  usewebkit=false ;;
		*) echo "bad value ${enableval} for --enable-webkit" ;;
	esac],[usewebkit=false])

if test "x$usewebkit" = "xtrue" ; then
	PKG_CHECK_MODULES(WEBKIT,webkit-1.0, [webkit_detected=true], [ webkit_detected=false; AC_MSG_WARN(Could not find webkit-1.0; Weblets plugin will try to use Gecko as backend.) ])
	AC_SUBST(WEBKIT_CFLAGS)
	AC_SUBST(WEBKIT_LIBS)
	
	if test ! "x$webkit_detected" = "xfalse" ; then
		AC_DEFINE(HAVE_WEBKIT,1,[Defined if we want to use webkit.])
	fi
fi

if test ! "x$webkit_detected" = "xtrue" ; then
  PKG_CHECK_MODULES(XULRUNNER, xulrunner-gtkmozembed, [gecko_detected=true], [ gecko_detected=false; AC_MSG_ERROR(Could not find xulrunner-gtkmozembed; Weblets plugin needs either this package or webkit-1.0 to work.) ])  
	AC_SUBST(XULRUNNER_CFLAGS)
	AC_SUBST(XULRUNNER_LIBS)
fi

AM_CONDITIONAL( [WITH_WEBKIT], [test "x$usewebkit" = "xtrue"] )


AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/weblets.conf])
AC_OUTPUT
