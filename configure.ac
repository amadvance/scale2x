# Process this file with autoconf to produce a configure script.
AC_INIT([Scale2x],[1.6])
AC_CONFIG_SRCDIR([scalerx.c])
AM_INIT_AUTOMAKE([foreign no-dependencies])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([m], [log])
AC_CHECK_LIB([z], [adler32])
AC_CHECK_LIB([png], [png_get_IHDR])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h getopt.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_ALLOCA
AC_CHECK_FUNCS([getopt getopt_long])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

