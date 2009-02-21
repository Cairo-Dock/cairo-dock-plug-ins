#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-slider],[1.0.3],[ChAnGFu (changfu@hollowproject.org)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_SLIDER=cd-slider
AC_DEFINE_UNQUOTED(GETTEXT_SLIDER,"$GETTEXT_SLIDER", [Gettext package.])
AC_SUBST(GETTEXT_SLIDER)

AC_SUBST(VERSION_SLIDER, "$VERSION")
AC_SUBST(PACKAGE_SLIDER, "$PACKAGE")

sliderdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/slider]
AC_SUBST(sliderdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)


PKG_CHECK_MODULES(PACKAGE, cairo-dock)

AC_ARG_ENABLE(exif,
	[  --enable-exif  :  for auto-rotating pictures],
	[case "${enableval}" in
		yes) useexif=true ;;
		no)  useexif=false ;;
		*) echo "bad value ${enableval} for --enable-exif" ;;
	esac],[useexif=true])

if test "$useexif" = "true" ; then
	PKG_CHECK_MODULES(EXIF, libexif, [exif_detected=true], [ exif_detected=false; AC_MSG_WARN(Could not find exif; Slider won't be built with exif capbilities.) ])
	AC_SUBST(EXIF_CFLAGS)
	AC_SUBST(EXIF_LIBS)
	
	if test ! "$exif_detected" = "false" ; then
		AC_DEFINE(HAVE_EXIF,"1",[Defined if we can use exif.])
	fi
fi

AM_CONDITIONAL( [WITH_EXIF], [test "$exif_detected" = "true"] )

AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/readme
		data/slider.conf])
AC_OUTPUT
