#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-mail],[1.0.0-beta],[Christophe Chapuis (chris.chapuis@gmail.com)])
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


# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpopt':

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h fcntl.h math.h netdb.h netinet/in.h signal.h stddef.h string.h sys/select.h sys/socket.h sys/stat.h sys/types.h sys/wait.h time.h unistd.h)
AC_CHECK_FUNCS(mmap sigaction srandom)

# Check for libetpan header files
AC_CHECK_HEADERS(libetpan/libetpan.h)

# Checks for libetpan library
AC_CHECK_LIB(etpan, mailstorage_connect)


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T

# Checks for library functions.
AC_CHECK_FUNCS([sscanf])

# Checks for crypt library
AC_CHECK_LIB(crypt, encrypt)

PKG_CHECK_MODULES(PACKAGE, cairo-dock)

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
