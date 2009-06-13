# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([cubby], m4_include([version.m4]), [kr@xph.us])
AC_CONFIG_SRCDIR([cubbyd.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([autostuff])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
# Let's not use AM_MAINTAINER_MODE.

AS_COMPILER_FLAGS(CUBBY_CFLAGS, "-Wall -Werror")

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
CUBBY_ENABLE_GCOV

AC_ARG_WITH(event,
  [AS_HELP_STRING([--with-event],
    [install prefix for libevent @<:@default is yes@:>@])],
  [],
  [with_event=yes])

test "x$with_event" = xno && AC_MSG_ERROR([Cubby requires libevent.])

CUBBY_HUNT_AROUND([event], [event_assign], [event2/event.h],
  [],
  [AC_MSG_ERROR([Cubby requires libevent 2.0 or later.])])

# Checks for header files.
#AC_CHECK_HEADERS([])

# Checks for typedefs, structures, and compiler characteristics.
#AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_CHECK_FUNC([socket], [], [AC_MSG_ERROR([Cubby requires socket])])
AC_CHECK_FUNC([strerror], [], [AC_MSG_ERROR([Cubby requires strerror])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
