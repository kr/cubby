
dnl autostars m4 macro for detection of compiler flags

dnl Keith Rarick <kr@xph.us>

dnl AS_HUNT_AROUND()
dnl Try to find a header for library in common locations.

AC_DEFUN([AS_HUNT_AROUND],
[
  AC_MSG_CHECKING([for $1 library prefix])
  if test "x$with_$1" = xyes
  then

    # TODO make this configurable
    for f in "/usr /usr/local /opt /local"
    do
      if test -f "$f/include/$1.h"
      then
        with_$1="$f"
        break
      fi
    done
    if test "x$with_$1" = xyes
    then
      AC_MSG_RESULT([no])
    else
      AC_MSG_RESULT([(found) $with_$1])
      LDFLAGS="$LDFLAGS -L$with_$1/lib"
      CFLAGS="$CFLAGS -I$with_$1/include"
    fi
  else
    AC_MSG_RESULT([(given) $with_$1])
    LDFLAGS="$LDFLAGS -L$with_$1/lib"
    CFLAGS="$CFLAGS -I$with_$1/include"
  fi

  AC_CHECK_LIB([$1], [$2], [$3], [$4])
])
