dnl @* AX_PATH_LIB_PCRE [(A/NA)]
dnl
dnl check for pcre lib and set PCRE_LIBS and PCRE_CFLAGS accordingly.
dnl 
dnl also provide --with-pcre option that may point to the $prefix of
dnl the pcre installation - the macro will check $pcre/include and
dnl $pcre/lib to contain the necessary files.
dnl
dnl the usual two ACTION-IF-FOUND / ACTION-IF-NOT-FOUND are supported
dnl and they can take advantage of the LIBS/CFLAGS additions.
dnl @: guidod@gmx.de
dnl @$Id: $


AC_DEFUN([AX_PATH_LIB_PCRE], [dnl
AC_MSG_HEADER([Configuring for libpcre now])
AX_CONFIG_LIB_PCRE
])
AC_DEFUN([AX_PATH_LIB_PCRE_NOTUSED], [dnl
AC_ARG_WITH(pcre,
	[  --with-pcre[[=prefix]]    compile xmlpcre part (via libpcre check)],,
	     with_pcre="yes")
if test ".$with_pcre" = ".no" ; then
  AC_MSG_RESULT([disabled])
  m4_ifval($2,$2)
  AX_CONFIG_LIB_PCRE
else
  AC_MSG_RESULT([(testing)])
  OLDLIBS="$LIBS"
  AC_CHECK_LIB(pcre, pcre_study)
  if test "$ac_cv_lib_pcre_pcre_study" = "yes" ; then
     PCRE_LIBS="-lpcre"
     PCRE_CFLAGS='-I$(top_srcdir)'/lib/pcre
     LIBS="$OLDLIBS"
     AC_MSG_CHECKING([lib pcre])
     AC_MSG_RESULT([$PCRE_LIBS])
     m4_ifval($1,$1)
     AM_CONDITIONAL(BUILD_PCRE, false)
     AC_CHECK_HEADERS(pcre.h)
     if eval pcre-config --cflags 2>/dev/null >/dev/null; then
	PCRE_CFLAGS=`pcre-config --cflags`
	PCRE_LIBS=`pcre-config --libs`
     fi
  else
     OLDLDFLAGS="$LDFLAGS" ; LDFLAGS="$LDFLAGS -L$with_pcre/lib"
     OLDCPPFLAGS="$CPPFLAGS" ; CPPFLAGS="$CPPFLAGS -I$with_pcre/include"
     OLDLIBS="$LIBS"
     AC_CHECK_LIB(pcre, pcre_compile)
     LIBS="$OLDLIBS"
     CPPFLAGS="$OLDCPPFLAGS"
     LDFLAGS="$OLDLDFLAGS"
     if test "$ac_cv_lib_pcre_pcre_compile" = "yes" ; then
        AC_MSG_RESULT(setting PCRE_LIBS -L$with_pcre/lib -lpcre)
        PCRE_LIBS="-L$with_pcre/lib -lpcre"
        test -d "$with_pcre/include" && PCRE_CFLAGS="-I$with_pcre/include"
	AC_CHECK_HEADERS(pcre.h)
        AC_MSG_CHECKING([lib pcre])
        AC_MSG_RESULT([$PCRE_LIBS])
        m4_ifval($1,$1)
	AM_CONDITIONAL(BUILD_PCRE, false)
     else
        AC_MSG_CHECKING([lib pcre])
        AC_MSG_RESULT([no, (WARNING)])
        m4_ifval($2,$2)
	AX_CONFIG_LIB_PCRE
     fi
  fi
fi 
AC_SUBST([PCRE_LIBS])
AC_SUBST([PCRE_CFLAGS])
])

AC_DEFUN([AX_CONFIG_LIB_PCRE],[dnl
dnl PCRE options
NPCRE_MAJOR_VERSION=4
NPCRE_MINOR_VERSION=3
NPCRE_VERSION=$PCRE_MAJOR_VERSION.$PCRE_MINOR_VERSION

AC_DEFINE(LINK_SIZE, 2, "PCRE Lnk size")
AC_DEFINE(POSIX_MALLOC_THRESHOLD, 5, "Malloc Threshold")
AC_DEFINE(MATCH_LIMIT, 10000000, "PCRE Match limit")
AC_DEFINE(USE_LOCAL_PCRE, 1, "Use the local copy of PCRE")
AC_DEFINE(PCRE_MAJOR_VERSION, $NPCRE_MAJOR_VERSION, "PCRE Version")
AC_DEFINE(PCRE_MINOR_VERSION, NPCRE_MINOR_VERSION, "PCRE Version")
AC_DEFINE(PCRE_VERSION, NPCRE_VERSION, "Pcre full version")
PCRE_LIBS='${top_srcdir}/'lib/pcre/libpcre.la
PCRE_CFLAGS='-I${top_srcdir}/'lib/pcre
AC_SUBST([PCRE_LIBS])
AC_SUBST([PCRE_CFLAGS])
AM_CONDITIONAL(BUILD_PCRE, true)
])
