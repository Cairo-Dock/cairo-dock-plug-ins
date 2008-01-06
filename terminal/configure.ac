#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cd-terminal],[0.0.4],[GESTES Cedric (ctaf42@gmail.com)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_LIBTOOL


pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdir`/systray]
AC_SUBST(pkgdatadir)


# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpopt':

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T

# Checks for library functions.
AC_CHECK_FUNCS([sscanf])

PKG_CHECK_MODULES(PACKAGE, cairo-dock)
PKG_CHECK_MODULES(LIBVTE, vte,, \
	AC_MSG_ERROR("*** vte not found! You need it to build $PACKAGE_NAME. ***"))
AC_SUBST(LIBVTE_CFLAGS)
AC_SUBST(LIBVTE_LIBS)

AC_CONFIG_FILES([Makefile
		src/Makefile
		data/Makefile])
AC_OUTPUT
