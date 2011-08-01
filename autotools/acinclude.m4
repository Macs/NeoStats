AC_DEFUN([TYPE_SOCKLEN_T],
[
   AC_CHECK_TYPE([socklen_t], ,[
      AC_MSG_CHECKING([for socklen_t equivalent])
      AC_CACHE_VAL([curl_cv_socklen_t_equiv],
      [
         # Systems have either "struct sockaddr *" or
         # "void *" as the second argument to getpeername
         curl_cv_socklen_t_equiv=
         for arg2 in "struct sockaddr" void; do
            for t in int size_t unsigned long "unsigned long"; do
               AC_TRY_COMPILE([
                  #ifdef HAVE_SYS_TYPES_H
                  #include <sys/types.h>
                  #endif
                  #ifdef HAVE_SYS_SOCKET_H
                  #include <sys/socket.h>
                  #endif

                  int getpeername (int, $arg2 *, $t *);
               ],[
                  $t len;
                  getpeername(0,0,&len);
               ],[
                  curl_cv_socklen_t_equiv="$t"
                  break
               ])
            done
         done

         if test "x$curl_cv_socklen_t_equiv" = x; then
            AC_MSG_ERROR([Cannot find a type to use in place of socklen_t])
         fi
      ])
      AC_MSG_RESULT($curl_cv_socklen_t_equiv)
      AC_DEFINE_UNQUOTED(socklen_t, $curl_cv_socklen_t_equiv,
			[type to use in place of socklen_t if not defined])],
      [#include <sys/types.h>
#include <sys/socket.h>])
])

dnl Check for in_addr_t: it is used to receive the return code of inet_addr()
dnl and a few other things. If not found, we set it to unsigned int, as even
dnl 64-bit implementations use to set it to a 32-bit type.
AC_DEFUN([TYPE_IN_ADDR_T],
[
   AC_CHECK_TYPE([in_addr_t], ,[
      AC_MSG_CHECKING([for in_addr_t equivalent])
      AC_CACHE_VAL([curl_cv_in_addr_t_equiv],
      [
         curl_cv_in_addr_t_equiv=
         for t in "unsigned long" int size_t unsigned long; do
            AC_TRY_COMPILE([
               #ifdef HAVE_SYS_TYPES_H
               #include <sys/types.h>
               #endif
               #ifdef HAVE_SYS_SOCKET_H
               #include <sys/socket.h>
               #endif
               #ifdef HAVE_ARPA_INET_H
               #include <arpa/inet.h>
               #endif
            ],[
               $t data = inet_addr ("1.2.3.4");
            ],[
               curl_cv_in_addr_t_equiv="$t"
               break
            ])
         done

         if test "x$curl_cv_in_addr_t_equiv" = x; then
            AC_MSG_ERROR([Cannot find a type to use in place of in_addr_t])
         fi
      ])
      AC_MSG_RESULT($curl_cv_in_addr_t_equiv)
      AC_DEFINE_UNQUOTED(in_addr_t, $curl_cv_in_addr_t_equiv,
			[type to use in place of in_addr_t if not defined])],
      [#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>])
])
AC_DEFUN([CURL_CHECK_NONBLOCKING_SOCKET],
[
  AC_MSG_CHECKING([non-blocking sockets style])

  AC_TRY_COMPILE([
/* headers for O_NONBLOCK test */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
],[
/* try to compile O_NONBLOCK */

#if defined(sun) || defined(__sun__) || defined(__SUNPRO_C) || defined(__SUNPRO_CC)
# if defined(__SVR4) || defined(__srv4__)
#  define PLATFORM_SOLARIS
# else
#  define PLATFORM_SUNOS4
# endif
#endif
#if (defined(_AIX) || defined(__xlC__)) && !defined(_AIX4)
# define PLATFORM_AIX_V3
#endif

#if defined(PLATFORM_SUNOS4) || defined(PLATFORM_AIX_V3) || defined(__BEOS__)
#error "O_NONBLOCK does not work on this platform"
#endif
  int socket;
  int flags = fcntl(socket, F_SETFL, flags | O_NONBLOCK);
],[
dnl the O_NONBLOCK test was fine
nonblock="O_NONBLOCK"
AC_DEFINE(HAVE_O_NONBLOCK, 1, [use O_NONBLOCK for non-blocking sockets])
],[
dnl the code was bad, try a different program now, test 2

  AC_TRY_COMPILE([
/* headers for FIONBIO test */
#include <unistd.h>
#include <stropts.h>
],[
/* FIONBIO source test (old-style unix) */
 int socket;
 int flags = ioctl(socket, FIONBIO, &flags);
],[
dnl FIONBIO test was good
nonblock="FIONBIO"
AC_DEFINE(HAVE_FIONBIO, 1, [use FIONBIO for non-blocking sockets])
],[
dnl FIONBIO test was also bad
dnl the code was bad, try a different program now, test 3

  AC_TRY_COMPILE([
/* headers for ioctlsocket test (cygwin?) */
#include <windows.h>
],[
/* ioctlsocket source code */
 int socket;
 int flags = ioctlsocket(socket, FIONBIO, &flags);
],[
dnl ioctlsocket test was good
nonblock="ioctlsocket"
AC_DEFINE(HAVE_IOCTLSOCKET, 1, [use ioctlsocket() for non-blocking sockets])
],[
dnl ioctlsocket didnt compile!, go to test 4

  AC_TRY_LINK([
/* headers for IoctlSocket test (Amiga?) */
#include <sys/ioctl.h>
],[
/* IoctlSocket source code */
 int socket;
 int flags = IoctlSocket(socket, FIONBIO, (long)1);
],[
dnl ioctlsocket test was good
nonblock="IoctlSocket"
AC_DEFINE(HAVE_IOCTLSOCKET_CASE, 1, [use Ioctlsocket() for non-blocking sockets])
],[
dnl Ioctlsocket didnt compile, do test 5!
  AC_TRY_COMPILE([
/* headers for SO_NONBLOCK test (BeOS) */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
],[
/* SO_NONBLOCK source code */
 long b = 1;
 int socket;
 int flags = setsockopt(socket, SOL_SOCKET, SO_NONBLOCK, &b, sizeof(b));
],[
dnl the SO_NONBLOCK test was good
nonblock="SO_NONBLOCK"
AC_DEFINE(HAVE_SO_NONBLOCK, 1, [use SO_NONBLOCK for non-blocking sockets])
],[
dnl test 5 didnt compile!
nonblock="nada"
AC_DEFINE(HAVE_DISABLED_NONBLOCKING, 1, [disabled non-blocking sockets])
])
dnl end of fifth test

])
dnl end of forth test

])
dnl end of third test

])
dnl end of second test

])
dnl end of non-blocking try-compile test
  AC_MSG_RESULT($nonblock)

  if test "$nonblock" = "nada"; then
    AC_MSG_WARN([non-block sockets disabled])
  fi
])

dnl DPKG_CACHED_TRY_COMPILE(<description>,<cachevar>,<include>,<program>,<ifyes>,<ifno>)
define(DPKG_CACHED_TRY_COMPILE,[
 AC_MSG_CHECKING($1)
 AC_CACHE_VAL($2,[
  AC_TRY_COMPILE([$3],[$4],[$2=yes],[$2=no])
 ])
 if test "x$$2" = xyes; then
  true
  $5
 else
  true
  $6
 fi
])



define(ADNS_C_GCCATTRIB,[
 DPKG_CACHED_TRY_COMPILE(__attribute__((,,)),adns_cv_c_attribute_supported,,
  [extern int testfunction(int x) __attribute__((,,))],
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_GNUC25_ATTRIB, 1, HAVE_GNUC25_ATTRIB)
   DPKG_CACHED_TRY_COMPILE(__attribute__((noreturn)),adns_cv_c_attribute_noreturn,,
    [extern int testfunction(int x) __attribute__((noreturn))],
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_GNUC25_NORETURN, 1, HAVE_GNUC25_NORETURN),
    AC_MSG_RESULT(no))
   DPKG_CACHED_TRY_COMPILE(__attribute__((const)),adns_cv_c_attribute_const,,
    [extern int testfunction(int x) __attribute__((const))],
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_GNUC25_CONST, 1, HAVE_GNUC25_CONST),
    AC_MSG_RESULT(no))
   DPKG_CACHED_TRY_COMPILE(__attribute__((format...)),adns_cv_attribute_format,,
    [extern int testfunction(char *y, ...) __attribute__((format(printf,1,2)))],
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_GNUC25_PRINTFFORMAT, 1, HAVE_GNUC25_PRINTFFORMAT),
    AC_MSG_RESULT(no)),
  AC_MSG_RESULT(no))
])

define(ADNS_C_GETFUNC,[
 AC_CHECK_FUNC([$1],,[
  AC_CHECK_LIB([$2],[$1],[$3],[
    AC_MSG_ERROR([cannot find library function $1])
  ])
 ])
])
AC_DEFUN([AC_MSG_HEADER], [
echo ""
$srcdir/autotools/shtool echo -e "%B$1%b"
echo ""
])
# Prevent libtool for checking how to run C++ compiler and check for other
# tools we don't want to use. We do this by m4-defining the _LT_AC_TAGCONFIG
# variable to the code to run, as by default it uses a much more complicated
# approach. The code below that is actually added seems to be used for cases
# where configure has trouble figuring out what C compiler to use but where
# the installed libtool has an idea.
#
# This function is a re-implemented version of the Paolo Bonzini fix posted to
# the c-ares mailing list by Bram Matthys on May 6 2006. My version removes
# redundant code but also adds the LTCFLAGS check that wasn't in that patch.
#
# Some code in this function was extracted from the generated configure script.
#
# CARES_CLEAR_LIBTOOL_TAGS
AC_DEFUN([CARES_CLEAR_LIBTOOL_TAGS],
  [m4_define([_LT_AC_TAGCONFIG], [
  if test -f "$ltmain"; then
    if test ! -f "${ofile}"; then
      AC_MSG_WARN([output file `$ofile' does not exist])
    fi

    if test -z "$LTCC"; then
      eval "`$SHELL ${ofile} --config | grep '^LTCC='`"
      if test -z "$LTCC"; then
        AC_MSG_WARN([output file `$ofile' does not look like a libtool
script])
      else
        AC_MSG_WARN([using `LTCC=$LTCC', extracted from `$ofile'])
      fi
    fi
    if test -z "$LTCFLAGS"; then
      eval "`$SHELL ${ofile} --config | grep '^LTCFLAGS='`"
    fi
  fi
  ])]
)

