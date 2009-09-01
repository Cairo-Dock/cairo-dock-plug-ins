#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-mail],[1.0.0-beta0],[Christophe Chapuis (chris.chapuis@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT
GETTEXT_MAIL=cd-mail
AC_DEFINE_UNQUOTED(GETTEXT_MAIL,"$GETTEXT_MAIL", [Gettext mail package.])
AC_SUBST(GETTEXT_MAIL)

AC_SUBST(VERSION_MAIL, "$VERSION")
AC_SUBST(PACKAGE_MAIL, "$PACKAGE")

maildatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`/mail]
AC_SUBST(maildatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)
posubdir=po
AC_SUBST(posubdir)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)
MAIL_PACKAGE_CFLAGS=`libetpan-config --cflags`
MAIL_PACKAGE_LIBS=`libetpan-config --libs`
if test "x$MAIL_PACKAGE_LIBS" = "x"; then
	AC_MSG_ERROR("*** libetpan not found! You need it to build $PACKAGE_NAME. ***")
fi
AC_SUBST(MAIL_PACKAGE_CFLAGS)
AC_SUBST(MAIL_PACKAGE_LIBS)

# SSL is present ? let's say yes :)
AC_DEFINE_UNQUOTED([HAVE_SSL_SUPPORT], [1], [Define if SSL support is enabled])


AC_CONFIG_FILES([Makefile
		src/Makefile
		po/Makefile.in
		data/Makefile
		data/themes/Makefile
		data/themes/Default/Makefile
		data/mail.conf])
AC_OUTPUT
