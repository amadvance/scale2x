# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.65])
AC_INIT([Scale2x], [4.0], [], [], [http://www.scale2x.it])
AM_INIT_AUTOMAKE([foreign no-dependencies])
AC_CONFIG_SRCDIR([scalerx.c])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AC_CHECK_PROG([WINE],[wine],[wine],[])
AC_CHECK_PROG([ADVD2],[advd2],[advd2],[])
AM_CONDITIONAL(HAVE_ADVD2, [test x"$ADVD2" != x])

# Checks for libraries.
AC_CHECK_LIB([m], [log], [], [AC_MSG_ERROR([the libm library is missing])])
AC_CHECK_LIB([z], [adler32], [], [AC_MSG_ERROR([the libz library is missing])])
AC_CHECK_LIB([png], [png_get_IHDR],
	[],
	# If not found, try finding it with pkg-config
	[CFLAGS="$CFLAGS $(pkg-config libpng --cflags)"]
	[LDFLAGS="$LDFLAGS $(pkg-config libpng --libs)"]
)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h getopt.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT

# Checks for library functions.
AC_FUNC_ALLOCA
AC_CHECK_FUNCS([getopt getopt_long])

dnl Checks for architecture
AC_C_BIGENDIAN

dnl Checks for compiler
AC_CHECK_CC_OPT([-msse2], CFLAGS="$CFLAGS -msse2", [])
AC_CHECK_CC_OPT([-mfpu=neon], CFLAGS="$CFLAGS -mfpu=neon", [])
AC_CHECK_CC_OPT([-Wall], CFLAGS="$CFLAGS -Wall", [])
AC_CHECK_CC_OPT([-Wextra], CFLAGS="$CFLAGS -Wextra", [])

dnl Checks for test environment
AS_CASE([$host],
	[*-*-mingw*], [TESTENV="$WINE"],
	[]
)

AC_ARG_VAR([TESTENV], [Test environment])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

