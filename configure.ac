# see semver.org for version info
AC_INIT([cconfigspace],[0.0.1],[bvideau@anl.gov])

# are we in the right source dir ?
AC_CONFIG_SRCDIR([src/cconfigspace.c])

# build artefacts in separate dir
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_MACRO_DIR([m4])

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror foreign 1.12])
AM_PROG_AR

# check for libtool
LT_INIT

# check for programs
AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_STDC
AC_TYPE_SIZE_T
AC_HEADER_ASSERT

#check for libraries
AC_CHECK_LIB([m],[cos])
AC_CHECK_LIB([gslcblas],[cblas_dgemm])
AC_CHECK_LIB([gsl],[gsl_blas_dgemm])

#check for valgrind
AX_VALGRIND_DFLT([helgrind], [off])
AX_VALGRIND_DFLT([drd], [off])
AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile
		 include/Makefile
		 src/Makefile
		 tests/Makefile
		 cconfigspace.pc])
AC_OUTPUT
