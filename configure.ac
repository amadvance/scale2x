# Process this file with autoconf to produce a configure script.
AC_INIT([Scale2x],[1.5])
AC_CONFIG_SRCDIR([scale2x.c])
AM_INIT_AUTOMAKE([foreign no-dependencies])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([z], [adler32])
AC_CHECK_LIB([png], [png_get_IHDR])

# Checks for header files.
AC_CHECK_HEADERS([getopt.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

