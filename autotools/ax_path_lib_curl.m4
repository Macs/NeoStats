#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 1998 - 2006, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://curl.haxx.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# $Id: acinclude.m4,v 1.113 2006-10-16 08:30:56 bagder Exp $
###########################################################################
AC_DEFUN([CURL_CHECK_SSL_ENABLE], [
dnl **********************************************************************
dnl Check for the presence of SSL libraries and headers
dnl **********************************************************************

dnl Default to compiler & linker defaults for SSL files & libraries.
OPT_SSL=off
dnl Default to no CA bundle
ca="no"
AC_ARG_WITH(ssl,dnl
AC_HELP_STRING([--with-ssl=PATH],[Where to look for OpenSSL, PATH points to the SSL installation (default: /usr/local/ssl); when possible, set the PKG_CONFIG_PATH environment variable instead of using this option])
AC_HELP_STRING([--without-ssl], [disable SSL]),
  OPT_SSL=$withval)

if test X"$OPT_SSL" != Xno; then
  dnl backup the pre-ssl variables
  CLEANLDFLAGS="$LDFLAGS"
  CLEANCPPFLAGS="$CPPFLAGS"
  CLEANLIBS="$LIBS"

  case "$OPT_SSL" in
  yes)
    dnl --with-ssl (without path) used
    if test x$cross_compiling != xyes; then
      dnl only do pkg-config magic when not cross-compiling
      PKGTEST="yes"
    fi
    PREFIX_OPENSSL=/usr/local/ssl
    LIB_OPENSSL="$PREFIX_OPENSSL/lib$libsuff"
    ;;
  off)
    dnl no --with-ssl option given, just check default places
    if test x$cross_compiling != xyes; then
      dnl only do pkg-config magic when not cross-compiling
      PKGTEST="yes"
    fi
    PREFIX_OPENSSL=
    ;;
  *)
    dnl check the given --with-ssl spot
    PKGTEST="no"
    PREFIX_OPENSSL=$OPT_SSL
    LIB_OPENSSL="$PREFIX_OPENSSL/lib$libsuff"
    LDFLAGS="$LDFLAGS -L$LIB_OPENSSL"
    CPPFLAGS="$CPPFLAGS -I$PREFIX_OPENSSL/include/openssl -I$PREFIX_OPENSSL/include"
    ;;
  esac

  if test "$PKGTEST" = "yes"; then

    dnl Detect the pkg-config tool, as it may have extra info about the
    dnl openssl installation we can use. I *believe* this is what we are
    dnl expected to do on really recent Redhat Linux hosts.

    AC_PATH_PROG( PKGCONFIG, pkg-config, no, $PATH:/usr/bin:/usr/local/bin)
    if test "$PKGCONFIG" != "no" ; then
      AC_MSG_CHECKING([OpenSSL options with pkg-config])

      $PKGCONFIG --exists openssl
      SSL_EXISTS=$?

      if test "$SSL_EXISTS" -eq "0"; then
        SSL_LIBS=`$PKGCONFIG --libs-only-l openssl 2>/dev/null`
        SSL_LDFLAGS=`$PKGCONFIG --libs-only-L openssl 2>/dev/null`
        SSL_CPPFLAGS=`$PKGCONFIG --cflags-only-I openssl 2>/dev/null`

	LIB_OPENSSL=`echo $SSL_LDFLAGS | sed -e 's/-L//g'`

        dnl use the values pkg-config reported
        LIBS="$LIBS $SSL_LIBS"
        CPPFLAGS="$CPPFLAGS $SSL_CPPFLAGS"
        LDFLAGS="$LDFLAGS $SSL_LDFLAGS"
        AC_MSG_RESULT([found])
      else
        AC_MSG_RESULT([no])
      fi
    fi
  fi

  dnl This is for Msys/Mingw
  case $host in    
    *-*-cygwin*)
      dnl Under Cygwin this is extraneous and causes an unnecessary -lgdi32
      dnl to be added to LIBS and recorded in the .la file.
      ;;
    *)
      AC_MSG_CHECKING([for gdi32])
      my_ac_save_LIBS=$LIBS
      LIBS="-lgdi32 $LIBS"
      AC_TRY_LINK([#include <windef.h>
                   #include <wingdi.h>],
                   [GdiFlush();],
                   [ dnl worked!
                   AC_MSG_RESULT([yes])],
                   [ dnl failed, restore LIBS
                   LIBS=$my_ac_save_LIBS
                   AC_MSG_RESULT(no)]
                  )
      ;;
  esac

  AC_CHECK_LIB(crypto, CRYPTO_lock,[
     HAVECRYPTO="yes"
     LIBS="-lcrypto $LIBS"
     ],[
     LDFLAGS="$CLEANLDFLAGS -L$LIB_OPENSSL"
     CPPFLAGS="$CLEANCPPFLAGS -I$PREFIX_OPENSSL/include/openssl -I$PREFIX_OPENSSL/include"
     AC_CHECK_LIB(crypto, CRYPTO_add_lock,[
       HAVECRYPTO="yes"
       LIBS="-lcrypto $LIBS"], [
       LDFLAGS="$CLEANLDFLAGS"
       CPPFLAGS="$CLEANCPPFLAGS"
       LIBS="$CLEANLIBS"
       ])
    ])


  if test X"$HAVECRYPTO" = X"yes"; then
    dnl This is only reasonable to do if crypto actually is there: check for
    dnl SSL libs NOTE: it is important to do this AFTER the crypto lib

    AC_CHECK_LIB(ssl, SSL_connect)

    if test "$ac_cv_lib_ssl_SSL_connect" != yes; then
        dnl we didn't find the SSL lib, try the RSAglue/rsaref stuff
        AC_MSG_CHECKING(for ssl with RSAglue/rsaref libs in use);
        OLIBS=$LIBS
        LIBS="$LIBS -lRSAglue -lrsaref"
        AC_CHECK_LIB(ssl, SSL_connect)
        if test "$ac_cv_lib_ssl_SSL_connect" != yes; then
            dnl still no SSL_connect
            AC_MSG_RESULT(no)
            LIBS=$OLIBS
        else
            AC_MSG_RESULT(yes)
        fi

    else

      dnl Have the libraries--check for SSLeay/OpenSSL headers
      AC_CHECK_HEADERS(openssl/x509.h openssl/rsa.h openssl/crypto.h \
                       openssl/pem.h openssl/ssl.h openssl/err.h,
        curl_ssl_msg="enabled (OpenSSL)"
        OPENSSL_ENABLED=1
        AC_DEFINE(USE_OPENSSL, 1, [if OpenSSL is in use]))

      if test $ac_cv_header_openssl_x509_h = no; then
        dnl we don't use the "action" part of the AC_CHECK_HEADERS macro
        dnl since 'err.h' might in fact find a krb4 header with the same
        dnl name
        AC_CHECK_HEADERS(x509.h rsa.h crypto.h pem.h ssl.h err.h)

        if test $ac_cv_header_x509_h = yes && 
           test $ac_cv_header_crypto_h = yes &&
           test $ac_cv_header_ssl_h = yes; then
          dnl three matches
          curl_ssl_msg="enabled (OpenSSL)"
          OPENSSL_ENABLED=1
        fi
      fi
    fi

    if test X"$OPENSSL_ENABLED" = X"1"; then
       AC_DEFINE(USE_SSLEAY, 1, [if SSL is enabled])

       dnl is there a pkcs12.h header present?
       AC_CHECK_HEADERS(openssl/pkcs12.h)
    else
       LIBS="$CLEANLIBS"
    fi
    dnl USE_SSLEAY is the historical name for what configure calls
    dnl OPENSSL_ENABLED; the names should really be unified
    USE_SSLEAY="$OPENSSL_ENABLED"
    AC_SUBST(USE_SSLEAY)

    if test X"$OPT_SSL" != Xoff &&
       test "$OPENSSL_ENABLED" != "1"; then
      AC_MSG_ERROR([OpenSSL libs and/or directories were not found where specified!])
    fi
  fi

  if test X"$OPENSSL_ENABLED" = X"1"; then
    dnl If the ENGINE library seems to be around, check for the OpenSSL engine
    dnl stuff, it is kind of "separated" from the main SSL check
    AC_CHECK_FUNC(ENGINE_init,
              [
                AC_CHECK_HEADERS(openssl/engine.h)
                AC_CHECK_FUNCS( ENGINE_load_builtin_engines )
              ])

    dnl these can only exist if openssl exists

    AC_CHECK_FUNCS( RAND_status \
                    RAND_screen \
                    RAND_egd \
                    CRYPTO_cleanup_all_ex_data )

  fi

  if test "$OPENSSL_ENABLED" = "1"; then
    if test -n "$LIB_OPENSSL"; then
       dnl when the ssl shared libs were found in a path that the run-time
       dnl linker doesn't search through, we need to add it to LD_LIBRARY_PATH
       dnl to prevent further configure tests to fail due to this

       LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$LIB_OPENSSL"
       export LD_LIBRARY_PATH
       AC_MSG_NOTICE([Added $LIB_OPENSSL to LD_LIBRARY_PATH])
    fi
  fi

fi

dnl **********************************************************************
dnl Check for the random seed preferences 
dnl **********************************************************************

if test X"$OPENSSL_ENABLED" = X"1"; then
  AC_ARG_WITH(egd-socket,
  AC_HELP_STRING([--with-egd-socket=FILE],
                 [Entropy Gathering Daemon socket pathname]),
      [ EGD_SOCKET="$withval" ]
  )
  if test -n "$EGD_SOCKET" ; then
          AC_DEFINE_UNQUOTED(EGD_SOCKET, "$EGD_SOCKET",
          [your Entropy Gathering Daemon socket pathname] )
  fi

  dnl Check for user-specified random device
  AC_ARG_WITH(random,
  AC_HELP_STRING([--with-random=FILE],
                 [read randomness from FILE (default=/dev/urandom)]),
      [ RANDOM_FILE="$withval" ],
      [
          dnl Check for random device
          AC_CHECK_FILE("/dev/urandom", [ RANDOM_FILE="/dev/urandom"] )
      ]
  )
  if test -n "$RANDOM_FILE" && test X"$RANDOM_FILE" != Xno ; then
          AC_SUBST(RANDOM_FILE)
          AC_DEFINE_UNQUOTED(RANDOM_FILE, "$RANDOM_FILE",
          [a suitable file to read random data from])
  fi
fi

dnl ----------------------------------------------------
dnl FIX: only check for GnuTLS if OpenSSL is not enabled
dnl ----------------------------------------------------

dnl Default to compiler & linker defaults for GnuTLS files & libraries.
OPT_GNUTLS=no

AC_ARG_WITH(gnutls,dnl
AC_HELP_STRING([--with-gnutls=PATH],[where to look for GnuTLS, PATH points to the installation root (default: /usr/local/)])
AC_HELP_STRING([--without-gnutls], [disable GnuTLS detection]),
  OPT_GNUTLS=$withval)

if test "$OPENSSL_ENABLED" != "1"; then

  if test X"$OPT_GNUTLS" != Xno; then
    if test "x$OPT_GNUTLS" = "xyes"; then
     check=`libgnutls-config --version 2>/dev/null`
     if test -n "$check"; then
       addlib=`libgnutls-config --libs`
       addcflags=`libgnutls-config --cflags`
       version=`libgnutls-config --version`
       gtlsprefix=`libgnutls-config --prefix`
     fi
    else
      addlib=`$OPT_GNUTLS/bin/libgnutls-config --libs`
      addcflags=`$OPT_GNUTLS/bin/libgnutls-config --cflags`
      version=`$OPT_GNUTLS/bin/libgnutls-config --version 2>/dev/null`
      gtlsprefix=$OPT_GNUTLS
      if test -z "$version"; then
        version="unknown"
      fi
    fi
    if test -n "$addlib"; then

      CLEANLIBS="$LIBS"
      CLEANCPPFLAGS="$CPPFLAGS"
  
      LIBS="$LIBS $addlib"
      if test "$addcflags" != "-I/usr/include"; then
         CPPFLAGS="$CPPFLAGS $addcflags"
      fi
  
      AC_CHECK_LIB(gnutls, gnutls_check_version,
       [
       AC_DEFINE(USE_GNUTLS, 1, [if GnuTLS is enabled])
       AC_SUBST(USE_GNUTLS, [1])
       USE_GNUTLS="yes"
       curl_ssl_msg="enabled (GnuTLS)"
       ],
       [
         LIBS="$CLEANLIBS"
         CPPFLAGS="$CLEANCPPFLAGS"
       ])
  
      if test "x$USE_GNUTLS" = "xyes"; then
        AC_MSG_NOTICE([detected GnuTLS version $version])

        dnl when shared libs were found in a path that the run-time
        dnl linker doesn't search through, we need to add it to
        dnl LD_LIBRARY_PATH to prevent further configure tests to fail
        dnl due to this

        LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$gtlsprefix/lib$libsuff"
        export LD_LIBRARY_PATH
        AC_MSG_NOTICE([Added $gtlsprefix/lib$libsuff to LD_LIBRARY_PATH])
      fi

    fi

  fi dnl GNUTLS not disabled

  if test X"$USE_GNUTLS" != "Xyes"; then
    AC_MSG_WARN([SSL disabled, you will not be able to use HTTPS, FTPS, NTLM and more.])
    AC_MSG_WARN([Use --with-ssl or --with-gnutls to address this.])
  fi

fi dnl OPENSSL != 1

dnl **********************************************************************
dnl Check for the CA bundle
dnl **********************************************************************

if test X"$USE_GNUTLS$OPENSSL_ENABLED" != "X"; then

  AC_MSG_CHECKING([CA cert bundle install path])

  AC_ARG_WITH(ca-bundle,
AC_HELP_STRING([--with-ca-bundle=FILE], [File name to install the CA bundle as])
AC_HELP_STRING([--without-ca-bundle], [Don't install the CA bundle]),
    [ ca="$withval" ],
    [
      if test "x$prefix" != xNONE; then
        ca="\${prefix}/share/curl/curl-ca-bundle.crt"
      else
        ca="$ac_default_prefix/share/curl/curl-ca-bundle.crt"
      fi
    ] )

    if test "x$ca" != "xno"; then
      CURL_CA_BUNDLE='"'$ca'"'
      AC_SUBST(CURL_CA_BUNDLE)  
    fi
    AC_MSG_RESULT([$ca])
fi dnl only done if some kind of SSL was enabled

AM_CONDITIONAL(CABUNDLE, test x$ca != xno)
])


AC_DEFUN([CURL_CHECK_ZIP_ENABLE], [
dnl **********************************************************************
dnl Check for the presence of ZLIB libraries and headers
dnl **********************************************************************

dnl Check for & handle argument to --with-zlib.

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS
AC_ARG_WITH(zlib,
AC_HELP_STRING([--with-zlib=PATH],[search for zlib in PATH])
AC_HELP_STRING([--without-zlib],[disable use of zlib]),
               [OPT_ZLIB="$withval"])

if test "$OPT_ZLIB" = "no" ; then
    AC_MSG_WARN([zlib disabled])
else
  if test "$OPT_ZLIB" = "yes" ; then
     OPT_ZLIB=""
  fi

  if test -z "$OPT_ZLIB" ; then
    dnl check for the lib first without setting any new path, since many
    dnl people have it in the default path

    AC_CHECK_LIB(z, inflateEnd,
                   dnl libz found, set the variable
                   [HAVE_LIBZ="1"],
                   dnl if no lib found, try /usr/local
                   [OPT_ZLIB="/usr/local"])

  fi

  dnl Add a nonempty path to the compiler flags
  if test -n "$OPT_ZLIB"; then
     CPPFLAGS="$CPPFLAGS -I$OPT_ZLIB/include"
     LDFLAGS="$LDFLAGS -L$OPT_ZLIB/lib$libsuff"
  fi

  AC_CHECK_HEADER(zlib.h,
    [
    dnl zlib.h was found
    HAVE_ZLIB_H="1"
    dnl if the lib wasn't found already, try again with the new paths
    if test "$HAVE_LIBZ" != "1"; then
      AC_CHECK_LIB(z, gzread,
                   [
                   dnl the lib was found!
                   HAVE_LIBZ="1"
                   ],
                   [ CPPFLAGS=$_cppflags
                   LDFLAGS=$_ldflags])
    fi
    ],
    [
      dnl zlib.h was not found, restore the flags
      CPPFLAGS=$_cppflags
      LDFLAGS=$_ldflags]
    )

  if test "$HAVE_LIBZ" = "1" && test "$HAVE_ZLIB_H" != "1"
  then
    AC_MSG_WARN([configure found only the libz lib, not the header file!])
  elif test "$HAVE_LIBZ" != "1" && test "$HAVE_ZLIB_H" = "1"
  then
    AC_MSG_WARN([configure found only the libz header file, not the lib!])
  elif test "$HAVE_LIBZ" = "1" && test "$HAVE_ZLIB_H" = "1"
  then
    dnl both header and lib were found!
    AC_SUBST(HAVE_LIBZ)
    AC_DEFINE(HAVE_ZLIB_H, 1, [if you have the zlib.h header file])
    AC_DEFINE(HAVE_LIBZ, 1, [if zlib is available])

    LIBS="$LIBS -lz"

    dnl replace 'HAVE_LIBZ' in the automake makefile.ams
    AMFIXLIB="1"
    AC_MSG_NOTICE([found both libz and libz.h header])
    curl_zlib_msg="enabled"
  fi
fi

dnl set variable for use in automakefile(s)
AM_CONDITIONAL(HAVE_LIBZ, test x"$AMFIXLIB" = x1)
])


AC_DEFUN([CURL_CHECK_IPV6_ENABLE], [
dnl **********************************************************************
dnl Checks for IPv6
dnl **********************************************************************

AC_MSG_CHECKING([whether to enable ipv6])
AC_ARG_ENABLE(ipv6,
AC_HELP_STRING([--enable-ipv6],[Enable ipv6 (with ipv4) support])
AC_HELP_STRING([--disable-ipv6],[Disable ipv6 support]),
[ case "$enableval" in
  no)
       AC_MSG_RESULT(no)
       ipv6=no
       ;;
  *)   AC_MSG_RESULT(yes)
       ipv6=yes
       ;;
  esac ],

  AC_TRY_RUN([ /* is AF_INET6 available? */
#include <sys/types.h>
#include <sys/socket.h>
main()
{
 if (socket(AF_INET6, SOCK_STREAM, 0) < 0)
   exit(1);
 else
   exit(0);
}
],
  AC_MSG_RESULT(yes)
  ipv6=yes,
  AC_MSG_RESULT(no)
  ipv6=no,
  AC_MSG_RESULT(no)
  ipv6=no
))

if test "$ipv6" = "yes"; then
  curl_ipv6_msg="enabled"
fi
])


dnl CURL_CHECK_HEADER_MALLOC
dnl -------------------------------------------------
dnl Check for compilable and valid malloc.h header,
dnl and check if it is needed even with stdlib.h

AC_DEFUN([CURL_CHECK_HEADER_MALLOC], [
  AC_CACHE_CHECK([for malloc.h], [ac_cv_header_malloc_h], [
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([
#include <malloc.h>
      ],[
        void *p = malloc(10);
        void *q = calloc(10,10);
        free(p);
        free(q);
      ])
    ],[
      ac_cv_header_malloc_h="yes"
    ],[
      ac_cv_header_malloc_h="no"
    ])
  ])
  if test "$ac_cv_header_malloc_h" = "yes"; then
    AC_DEFINE_UNQUOTED(HAVE_MALLOC_H, 1,
      [Define to 1 if you have the malloc.h header file.])
    #
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([
#include <stdlib.h>
      ],[
        void *p = malloc(10);
        void *q = calloc(10,10);
        free(p);
        free(q);
      ])
    ],[
      curl_cv_need_header_malloc_h="no"
    ],[
      curl_cv_need_header_malloc_h="yes"
    ])
    #
    case "$curl_cv_need_header_malloc_h" in
      yes)
        AC_DEFINE_UNQUOTED(NEED_MALLOC_H, 1,
          [Define to 1 if you need the malloc.h header file even with stdlib.h])
        ;;
    esac
  fi
])


dnl CURL_CHECK_TYPE_SOCKLEN_T
dnl -------------------------------------------------
dnl Check for existing socklen_t type, and provide
dnl an equivalent type if socklen_t not available

AC_DEFUN([CURL_CHECK_TYPE_SOCKLEN_T], [
  AC_CHECK_TYPE([socklen_t], ,[
    AC_CACHE_CHECK([for socklen_t equivalent], 
      [curl_cv_socklen_t_equiv], [
      curl_cv_socklen_t_equiv="unknown"
      for arg2 in "struct sockaddr" void; do
        for t in int size_t unsigned long "unsigned long"; do
          AC_COMPILE_IFELSE([
            AC_LANG_PROGRAM([
#undef inline
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif
              int getpeername (int, $arg2 *, $t *);
            ],[
              $t len=0;
              getpeername(0,0,&len);
            ])
          ],[
             curl_cv_socklen_t_equiv="$t"
             break 2
          ])
        done
      done
    ])
    case "$curl_cv_socklen_t_equiv" in
      unknown)
        AC_MSG_ERROR([Cannot find a type to use in place of socklen_t])
        ;;
      *)
        AC_DEFINE_UNQUOTED(socklen_t, $curl_cv_socklen_t_equiv,
          [type to use in place of socklen_t if not defined])
        ;;
    esac
  ],[
#undef inline
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
#endif
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif
  ])
])


dnl CURL_CHECK_FUNC_GETNAMEINFO
dnl -------------------------------------------------
dnl Test if the getnameinfo function is available, 
dnl and check the types of five of its arguments.
dnl If the function succeeds HAVE_GETNAMEINFO will be
dnl defined, defining the types of the arguments in
dnl GETNAMEINFO_TYPE_ARG1, GETNAMEINFO_TYPE_ARG2,
dnl GETNAMEINFO_TYPE_ARG46 and GETNAMEINFO_TYPE_ARG7,
dnl and also defining the type qualifier of first 
dnl argument in GETNAMEINFO_QUAL_ARG1.

AC_DEFUN([CURL_CHECK_FUNC_GETNAMEINFO], [
  AC_REQUIRE([CURL_CHECK_TYPE_SOCKLEN_T])dnl
  AC_CHECK_HEADERS(netdb.h)
  #
  AC_MSG_CHECKING([for getnameinfo])
  AC_LINK_IFELSE([
      AC_LANG_FUNC_LINK_TRY([getnameinfo])
    ],[
      AC_MSG_RESULT([yes])
      curl_cv_getnameinfo="yes"
    ],[
      AC_MSG_RESULT([no])
      curl_cv_getnameinfo="no"
  ])
  #
  if test "$curl_cv_getnameinfo" != "yes"; then
    AC_MSG_CHECKING([deeper for getnameinfo])
    AC_TRY_LINK([
      ],[
        getnameinfo();
      ],[
        AC_MSG_RESULT([yes])
        curl_cv_getnameinfo="yes"
      ],[
        AC_MSG_RESULT([but still no])
        curl_cv_getnameinfo="no"
    ])
  fi
  #
  if test "$curl_cv_getnameinfo" != "yes"; then
    AC_MSG_CHECKING([deeper and deeper for getnameinfo])
    AC_TRY_LINK([
#undef inline
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
#endif
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#endif
      ],[
        getnameinfo(0, 0, 0, 0, 0, 0, 0);
      ],[ 
        AC_MSG_RESULT([yes])
        curl_cv_getnameinfo="yes"
      ],[
        AC_MSG_RESULT([but still no])
        curl_cv_getnameinfo="no"
    ])
  fi
  #
  if test "$curl_cv_getnameinfo" = "yes"; then
    AC_CACHE_CHECK([types of arguments for getnameinfo],
      [curl_cv_func_getnameinfo_args], [
      curl_cv_func_getnameinfo_args="unknown"
      for gni_arg1 in 'struct sockaddr *' 'const struct sockaddr *' 'void *'; do
        for gni_arg2 in 'socklen_t' 'size_t' 'int'; do
          for gni_arg46 in 'size_t' 'int' 'socklen_t' 'unsigned int' 'DWORD'; do
            for gni_arg7 in 'int' 'unsigned int'; do
              AC_COMPILE_IFELSE([
                AC_LANG_PROGRAM([
#undef inline 
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#if (!defined(_WIN32_WINNT)) || (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h> 
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
#endif
#define GNICALLCONV WSAAPI
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#define GNICALLCONV
#endif
                  extern int GNICALLCONV getnameinfo($gni_arg1, $gni_arg2,
                                         char *, $gni_arg46,
                                         char *, $gni_arg46,
                                         $gni_arg7);
                ],[
                  $gni_arg2 salen=0;
                  $gni_arg46 hostlen=0;
                  $gni_arg46 servlen=0;
                  $gni_arg7 flags=0;
                  int res = getnameinfo(0, salen, 0, hostlen, 0, servlen, flags);
                ])
              ],[
                 curl_cv_func_getnameinfo_args="$gni_arg1,$gni_arg2,$gni_arg46,$gni_arg7"
                 break 4
              ])
            done
          done
        done
      done
    ]) # AC_CACHE_CHECK
    if test "$curl_cv_func_getnameinfo_args" = "unknown"; then
      AC_MSG_WARN([Cannot find proper types to use for getnameinfo args])
      AC_MSG_WARN([HAVE_GETNAMEINFO will not be defined])
    else
      gni_prev_IFS=$IFS; IFS=','
      set dummy `echo "$curl_cv_func_getnameinfo_args" | sed 's/\*/\*/g'`
      IFS=$gni_prev_IFS
      shift
      #
      gni_qual_type_arg1=$[1]
      #
      AC_DEFINE_UNQUOTED(GETNAMEINFO_TYPE_ARG2, $[2],
        [Define to the type of arg 2 for getnameinfo.])
      AC_DEFINE_UNQUOTED(GETNAMEINFO_TYPE_ARG46, $[3],
        [Define to the type of args 4 and 6 for getnameinfo.])
      AC_DEFINE_UNQUOTED(GETNAMEINFO_TYPE_ARG7, $[4],
        [Define to the type of arg 7 for getnameinfo.])
      #
      prev_sh_opts=$-
      #
      case $prev_sh_opts in
        *f*)
          ;;
        *)
          set -f
          ;;
      esac
      #
      case "$gni_qual_type_arg1" in
        const*)
          gni_qual_arg1=const
          gni_type_arg1=`echo $gni_qual_type_arg1 | sed 's/^const //'`
        ;;
        *)
          gni_qual_arg1=
          gni_type_arg1=$gni_qual_type_arg1
        ;;
      esac
      #
      AC_DEFINE_UNQUOTED(GETNAMEINFO_QUAL_ARG1, $gni_qual_arg1,
        [Define to the type qualifier of arg 1 for getnameinfo.])
      AC_DEFINE_UNQUOTED(GETNAMEINFO_TYPE_ARG1, $gni_type_arg1,
        [Define to the type of arg 1 for getnameinfo.])
      #
      case $prev_sh_opts in
        *f*)
          ;;
        *)
          set +f
          ;;
      esac
      #
      AC_DEFINE_UNQUOTED(HAVE_GETNAMEINFO, 1,
        [Define to 1 if you have the getnameinfo function.])
      ac_cv_func_getnameinfo="yes"
    fi
  fi
]) # AC_DEFUN


dnl TYPE_SOCKADDR_STORAGE
dnl -------------------------------------------------
dnl Check for struct sockaddr_storage. Most IPv6-enabled 
dnl hosts have it, but AIX 4.3 is one known exception.

AC_DEFUN([TYPE_SOCKADDR_STORAGE],
[
   AC_CHECK_TYPE([struct sockaddr_storage],
        AC_DEFINE(HAVE_STRUCT_SOCKADDR_STORAGE, 1,
                  [if struct sockaddr_storage is defined]), ,
   [
#undef inline
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#endif
   ])
])


dnl CURL_CHECK_NI_WITHSCOPEID
dnl -------------------------------------------------
dnl Check for working NI_WITHSCOPEID in getnameinfo()

AC_DEFUN([CURL_CHECK_NI_WITHSCOPEID], [
  AC_REQUIRE([CURL_CHECK_FUNC_GETNAMEINFO])dnl
  AC_REQUIRE([TYPE_SOCKADDR_STORAGE])dnl
  #
  AC_CACHE_CHECK([for working NI_WITHSCOPEID], 
    [ac_cv_working_ni_withscopeid], [
    AC_RUN_IFELSE([
      AC_LANG_PROGRAM([
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
      ],[
#if defined(NI_WITHSCOPEID) && defined(HAVE_GETNAMEINFO)
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE
        struct sockaddr_storage sa;
#else
        unsigned char sa[256];
#endif
        char hostbuf[NI_MAXHOST];
        int rc;
        GETNAMEINFO_TYPE_ARG2 salen = (GETNAMEINFO_TYPE_ARG2)sizeof(sa);
        GETNAMEINFO_TYPE_ARG46 hostlen = (GETNAMEINFO_TYPE_ARG46)sizeof(hostbuf);
        GETNAMEINFO_TYPE_ARG7 flags = NI_NUMERICHOST | NI_NUMERICSERV | NI_WITHSCOPEID;
        int fd = socket(AF_INET6, SOCK_STREAM, 0);
        if(fd < 0) {
          perror("socket()");
          return 1; /* Error creating socket */
        }
        rc = getsockname(fd, (GETNAMEINFO_TYPE_ARG1)&sa, &salen);
        if(rc) {
          perror("getsockname()");
          return 2; /* Error retrieving socket name */
        }
        rc = getnameinfo((GETNAMEINFO_TYPE_ARG1)&sa, salen, hostbuf, hostlen, NULL, 0, flags);
        if(rc) {
          printf("rc = %s\n", gai_strerror(rc));
          return 3; /* Error translating socket address */
        }
        return 0; /* Ok, NI_WITHSCOPEID works */
#else
        return 4; /* Error, NI_WITHSCOPEID not defined or no getnameinfo() */
#endif
      ]) # AC_LANG_PROGRAM
    ],[
      # Exit code == 0. Program worked.
      ac_cv_working_ni_withscopeid="yes"
    ],[
      # Exit code != 0. Program failed.
      ac_cv_working_ni_withscopeid="no"
    ],[
      # Program is not run when cross-compiling. So we assume
      # NI_WITHSCOPEID will work if we are able to compile it.
      AC_COMPILE_IFELSE([
        AC_LANG_PROGRAM([
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
        ],[
          unsigned int dummy= NI_NUMERICHOST | NI_NUMERICSERV | NI_WITHSCOPEID;
        ])
      ],[
        ac_cv_working_ni_withscopeid="yes"
      ],[
        ac_cv_working_ni_withscopeid="no"
      ]) # AC_COMPILE_IFELSE
    ]) # AC_RUN_IFELSE
  ]) # AC_CACHE_CHECK
  case "$ac_cv_working_ni_withscopeid" in
    yes)
      AC_DEFINE(HAVE_NI_WITHSCOPEID, 1,
        [Define to 1 if NI_WITHSCOPEID exists and works.])
      ;;
  esac
]) # AC_DEFUN


dnl CURL_CHECK_FUNC_RECV
dnl -------------------------------------------------
dnl Test if the socket recv() function is available, 
dnl and check its return type and the types of its 
dnl arguments. If the function succeeds HAVE_RECV 
dnl will be defined, defining the types of the arguments 
dnl in RECV_TYPE_ARG1, RECV_TYPE_ARG2, RECV_TYPE_ARG3 
dnl and RECV_TYPE_ARG4, defining the type of the function
dnl return value in RECV_TYPE_RETV.

AC_DEFUN([CURL_CHECK_FUNC_RECV], [
  #
  AC_MSG_CHECKING([for recv])
  AC_TRY_LINK([
#undef inline 
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif
    ],[
      recv(0, 0, 0, 0);
    ],[ 
      AC_MSG_RESULT([yes])
      curl_cv_recv="yes"
    ],[
      AC_MSG_RESULT([no])
      curl_cv_recv="no"
  ])
  #
  if test "$curl_cv_recv" = "yes"; then
    AC_CACHE_CHECK([types of arguments and return type for recv],
      [curl_cv_func_recv_args], [
      curl_cv_func_recv_args="unknown"
      for recv_retv in 'int' 'ssize_t'; do
        for recv_arg1 in 'int' 'ssize_t' 'SOCKET'; do
          for recv_arg2 in 'char *' 'void *'; do
            for recv_arg3 in 'size_t' 'int' 'socklen_t' 'unsigned int'; do
              for recv_arg4 in 'int' 'unsigned int'; do
                AC_COMPILE_IFELSE([
                  AC_LANG_PROGRAM([
#undef inline 
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#define RECVCALLCONV PASCAL
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#define RECVCALLCONV
#endif
                    extern $recv_retv RECVCALLCONV recv($recv_arg1, $recv_arg2, $recv_arg3, $recv_arg4);
                  ],[
                    $recv_arg1 s=0;
                    $recv_arg2 buf=0;
                    $recv_arg3 len=0;
                    $recv_arg4 flags=0;
                    $recv_retv res = recv(s, buf, len, flags);
                  ])
                ],[
                   curl_cv_func_recv_args="$recv_arg1,$recv_arg2,$recv_arg3,$recv_arg4,$recv_retv"
                   break 5
                ])
              done
            done
          done
        done
      done
    ]) # AC_CACHE_CHECK
    if test "$curl_cv_func_recv_args" = "unknown"; then
      AC_MSG_ERROR([Cannot find proper types to use for recv args])
    else
      recv_prev_IFS=$IFS; IFS=','
      set dummy `echo "$curl_cv_func_recv_args" | sed 's/\*/\*/g'`
      IFS=$recv_prev_IFS
      shift
      #
      AC_DEFINE_UNQUOTED(RECV_TYPE_ARG1, $[1],
        [Define to the type of arg 1 for recv.])
      AC_DEFINE_UNQUOTED(RECV_TYPE_ARG2, $[2],
        [Define to the type of arg 2 for recv.])
      AC_DEFINE_UNQUOTED(RECV_TYPE_ARG3, $[3],
        [Define to the type of arg 3 for recv.])
      AC_DEFINE_UNQUOTED(RECV_TYPE_ARG4, $[4],
        [Define to the type of arg 4 for recv.])
      AC_DEFINE_UNQUOTED(RECV_TYPE_RETV, $[5],
        [Define to the function return type for recv.])
      #
      AC_DEFINE_UNQUOTED(HAVE_RECV, 1,
        [Define to 1 if you have the recv function.])
      ac_cv_func_recv="yes"
    fi
  else
    AC_MSG_ERROR([Unable to link function recv])
  fi
]) # AC_DEFUN


dnl CURL_CHECK_FUNC_SEND
dnl -------------------------------------------------
dnl Test if the socket send() function is available, 
dnl and check its return type and the types of its 
dnl arguments. If the function succeeds HAVE_SEND 
dnl will be defined, defining the types of the arguments 
dnl in SEND_TYPE_ARG1, SEND_TYPE_ARG2, SEND_TYPE_ARG3 
dnl and SEND_TYPE_ARG4, defining the type of the function
dnl return value in SEND_TYPE_RETV, and also defining the 
dnl type qualifier of second argument in SEND_QUAL_ARG2.

AC_DEFUN([CURL_CHECK_FUNC_SEND], [
  #
  AC_MSG_CHECKING([for send])
  AC_TRY_LINK([
#undef inline 
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif
    ],[
      send(0, 0, 0, 0);
    ],[ 
      AC_MSG_RESULT([yes])
      curl_cv_send="yes"
    ],[
      AC_MSG_RESULT([no])
      curl_cv_send="no"
  ])
  #
  if test "$curl_cv_send" = "yes"; then
    AC_CACHE_CHECK([types of arguments and return type for send],
      [curl_cv_func_send_args], [
      curl_cv_func_send_args="unknown"
      for send_retv in 'int' 'ssize_t'; do
        for send_arg1 in 'int' 'ssize_t' 'SOCKET'; do
          for send_arg2 in 'char *' 'void *' 'const char *' 'const void *'; do
            for send_arg3 in 'size_t' 'int' 'socklen_t' 'unsigned int'; do
              for send_arg4 in 'int' 'unsigned int'; do
                AC_COMPILE_IFELSE([
                  AC_LANG_PROGRAM([
#undef inline 
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#define SENDCALLCONV PASCAL
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#define SENDCALLCONV
#endif
                    extern $send_retv SENDCALLCONV send($send_arg1, $send_arg2, $send_arg3, $send_arg4);
                  ],[
                    $send_arg1 s=0;
                    $send_arg3 len=0;
                    $send_arg4 flags=0;
                    $send_retv res = send(s, 0, len, flags);
                  ])
                ],[
                   curl_cv_func_send_args="$send_arg1,$send_arg2,$send_arg3,$send_arg4,$send_retv"
                   break 5
                ])
              done
            done
          done
        done
      done
    ]) # AC_CACHE_CHECK
    if test "$curl_cv_func_send_args" = "unknown"; then
      AC_MSG_ERROR([Cannot find proper types to use for send args])
    else
      send_prev_IFS=$IFS; IFS=','
      set dummy `echo "$curl_cv_func_send_args" | sed 's/\*/\*/g'`
      IFS=$send_prev_IFS
      shift
      #
      send_qual_type_arg2=$[2]
      #
      AC_DEFINE_UNQUOTED(SEND_TYPE_ARG1, $[1],
        [Define to the type of arg 1 for send.])
      AC_DEFINE_UNQUOTED(SEND_TYPE_ARG3, $[3],
        [Define to the type of arg 3 for send.])
      AC_DEFINE_UNQUOTED(SEND_TYPE_ARG4, $[4],
        [Define to the type of arg 4 for send.])
      AC_DEFINE_UNQUOTED(SEND_TYPE_RETV, $[5],
        [Define to the function return type for send.])
      #
      prev_sh_opts=$-
      #
      case $prev_sh_opts in
        *f*)
          ;;
        *)
          set -f
          ;;
      esac
      #
      case "$send_qual_type_arg2" in
        const*)
          send_qual_arg2=const
          send_type_arg2=`echo $send_qual_type_arg2 | sed 's/^const //'`
        ;;
        *)
          send_qual_arg2=
          send_type_arg2=$send_qual_type_arg2
        ;;
      esac
      #
      AC_DEFINE_UNQUOTED(SEND_QUAL_ARG2, $send_qual_arg2,
        [Define to the type qualifier of arg 2 for send.])
      AC_DEFINE_UNQUOTED(SEND_TYPE_ARG2, $send_type_arg2,
        [Define to the type of arg 2 for send.])
      #
      case $prev_sh_opts in
        *f*)
          ;;
        *)
          set +f
          ;;
      esac
      #
      AC_DEFINE_UNQUOTED(HAVE_SEND, 1,
        [Define to 1 if you have the send function.])
      ac_cv_func_send="yes"
    fi
  else
    AC_MSG_ERROR([Unable to link function send])
  fi
]) # AC_DEFUN


dnl CURL_CHECK_MSG_NOSIGNAL
dnl -------------------------------------------------
dnl Check for MSG_NOSIGNAL

AC_DEFUN([CURL_CHECK_MSG_NOSIGNAL], [
  AC_CACHE_CHECK([for MSG_NOSIGNAL], [ac_cv_msg_nosignal], [
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([
#undef inline 
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif
      ],[
        int flag=MSG_NOSIGNAL;
      ])
    ],[
      ac_cv_msg_nosignal="yes"
    ],[
      ac_cv_msg_nosignal="no"
    ])
  ])
  case "$ac_cv_msg_nosignal" in
    yes)
      AC_DEFINE_UNQUOTED(HAVE_MSG_NOSIGNAL, 1,
        [Define to 1 if you have the MSG_NOSIGNAL flag.])
      ;;
  esac
]) # AC_DEFUN


dnl CURL_CHECK_STRUCT_TIMEVAL
dnl -------------------------------------------------
dnl Check for timeval struct

AC_DEFUN([CURL_CHECK_STRUCT_TIMEVAL], [
  AC_REQUIRE([AC_HEADER_TIME])dnl
  AC_CACHE_CHECK([for struct timeval], [ac_cv_struct_timeval], [
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([
#undef inline 
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif
#else
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#endif
      ],[
        struct timeval ts;
        ts.tv_sec  = 0;
        ts.tv_usec = 0;
      ])
    ],[
      ac_cv_struct_timeval="yes"
    ],[
      ac_cv_struct_timeval="no"
    ])
  ])
  case "$ac_cv_struct_timeval" in
    yes)
      AC_DEFINE_UNQUOTED(HAVE_STRUCT_TIMEVAL, 1,
        [Define to 1 if you have the timeval struct.])
      ;;
  esac
]) # AC_DEFUN


dnl CURL_CHECK_NONBLOCKING_SOCKET
dnl -------------------------------------------------
dnl Check for how to set a socket to non-blocking state. There seems to exist
dnl four known different ways, with the one used almost everywhere being POSIX
dnl and XPG3, while the other different ways for different systems (old BSD,
dnl Windows and Amiga).
dnl
dnl There are two known platforms (AIX 3.x and SunOS 4.1.x) where the
dnl O_NONBLOCK define is found but does not work. This condition is attempted
dnl to get caught in this script by using an excessive number of #ifdefs...
dnl
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
/* headers for ioctlsocket test (Windows) */
#undef inline
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#endif
],[
/* ioctlsocket source code */
 SOCKET sd;
 unsigned long flags = 0;
 sd = socket(0, 0, 0);
 ioctlsocket(sd, FIONBIO, &flags);
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
#include <socket.h>
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


dnl TYPE_IN_ADDR_T
dnl -------------------------------------------------
dnl Check for in_addr_t: it is used to receive the return code of inet_addr()
dnl and a few other things.
AC_DEFUN([TYPE_IN_ADDR_T],
[
   AC_CHECK_TYPE([in_addr_t], ,[
      AC_MSG_CHECKING([for in_addr_t equivalent])
      AC_CACHE_VAL([curl_cv_in_addr_t_equiv],
      [
         curl_cv_in_addr_t_equiv=
         for t in "unsigned long" int size_t unsigned long; do
            AC_TRY_COMPILE([
#undef inline
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
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
      [
#undef inline
#ifdef HAVE_WINDOWS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#endif
  ]) dnl AC_CHECK_TYPE
]) dnl AC_DEFUN

dnl ************************************************************
dnl check for "localhost", if it doesn't exist, we can't do the
dnl gethostbyname_r tests!
dnl 

AC_DEFUN([CURL_CHECK_WORKING_RESOLVER],[
AC_MSG_CHECKING([if "localhost" resolves])
AC_TRY_RUN([
#include <string.h>
#include <sys/types.h>
#include <netdb.h>

int
main () {
struct hostent *h;
h = gethostbyname("localhost");
exit (h == NULL ? 1 : 0); }],[
      AC_MSG_RESULT(yes)],[
      AC_MSG_RESULT(no)
      AC_MSG_ERROR([can't figure out gethostbyname_r() since localhost doesn't resolve])

      ]
)
])

dnl ************************************************************
dnl check for working getaddrinfo() that works with AI_NUMERICHOST
dnl
AC_DEFUN([CURL_CHECK_WORKING_GETADDRINFO],[
  AC_CACHE_CHECK(for working getaddrinfo, ac_cv_working_getaddrinfo,[
  AC_TRY_RUN( [
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(void)
{
    struct addrinfo hints, *ai;
    int error;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo("127.0.0.1", "8080", &hints, &ai);
    if (error) {
        return 1;
    }
    return 0;
}
],[
  ac_cv_working_getaddrinfo="yes"
],[
  ac_cv_working_getaddrinfo="no"
],[
  ac_cv_working_getaddrinfo="yes"
])])
if test "$ac_cv_working_getaddrinfo" = "yes"; then
  AC_DEFINE(HAVE_GETADDRINFO, 1, [Define if getaddrinfo exists and works])
  AC_DEFINE(ENABLE_IPV6, 1, [Define if you want to enable IPv6 support])

  IPV6_ENABLED=1
  AC_SUBST(IPV6_ENABLED)
fi
])


AC_DEFUN([CURL_CHECK_LOCALTIME_R],
[
  dnl check for localtime_r
  AC_CHECK_FUNCS(localtime_r,[
    AC_MSG_CHECKING(whether localtime_r is declared)
    AC_EGREP_CPP(localtime_r,[
#include <time.h>],[
      AC_MSG_RESULT(yes)],[
      AC_MSG_RESULT(no)
      AC_MSG_CHECKING(whether localtime_r with -D_REENTRANT is declared)
      AC_EGREP_CPP(localtime_r,[
#define _REENTRANT
#include <time.h>],[
	AC_DEFINE(NEED_REENTRANT)
	AC_MSG_RESULT(yes)],
	AC_MSG_RESULT(no))])])
])

dnl
dnl This function checks for strerror_r(). If it isn't found at first, it
dnl retries with _THREAD_SAFE defined, as that is what AIX seems to require
dnl in order to find this function.
dnl
dnl If the function is found, it will then proceed to check how the function
dnl actually works: glibc-style or POSIX-style.
dnl
dnl glibc:
dnl      char *strerror_r(int errnum, char *buf, size_t n);
dnl  
dnl  What this one does is to return the error string (no surprises there),
dnl  but it doesn't usually copy anything into buf!  The 'buf' and 'n'
dnl  parameters are only meant as an optional working area, in case strerror_r
dnl  needs it.  A quick test on a few systems shows that it's generally not
dnl  touched at all.
dnl
dnl POSIX:
dnl      int strerror_r(int errnum, char *buf, size_t n);
dnl
AC_DEFUN([CURL_CHECK_STRERROR_R],
[
  AC_CHECK_FUNCS(strerror_r)

  if test "x$ac_cv_func_strerror_r" = "xyes"; then

    AC_MSG_CHECKING(whether strerror_r is declared)
    AC_EGREP_CPP(strerror_r,[
#include <string.h>],[
      AC_MSG_RESULT(yes)],[
      AC_MSG_RESULT(no)
      AC_MSG_CHECKING(whether strerror_r with -D_REENTRANT is declared)
      AC_EGREP_CPP(strerror_r,[
#define _REENTRANT
#include <string.h>],[
	CPPFLAGS="-D_REENTRANT $CPPFLAGS"
	AC_MSG_RESULT(yes)],
	AC_MSG_RESULT(no)
        AC_DEFINE(HAVE_NO_STRERROR_R_DECL, 1, [we have no strerror_r() proto])
       ) dnl with _THREAD_SAFE
    ]) dnl plain cpp for it

    dnl determine if this strerror_r() is glibc or POSIX
    AC_MSG_CHECKING([for a glibc strerror_r API])
    AC_TRY_RUN([
#include <string.h>
#include <errno.h>
int
main () {
  char buffer[1024]; /* big enough to play with */
  char *string =
    strerror_r(EACCES, buffer, sizeof(buffer));
    /* this should've returned a string */
    if(!string || !string[0])
      return 99;
    return 0;
}
],
    GLIBC_STRERROR_R="1"
    AC_DEFINE(HAVE_GLIBC_STRERROR_R, 1, [we have a glibc-style strerror_r()])
    AC_MSG_RESULT([yes]),
    AC_MSG_RESULT([no]),

    dnl Use an inferior method of strerror_r detection while cross-compiling
    AC_EGREP_CPP(yes, [
#include <features.h>
#ifdef __GLIBC__
yes
#endif
], 
      dnl looks like glibc, so assume a glibc-style strerror_r()
      GLIBC_STRERROR_R="1"
      AC_DEFINE(HAVE_GLIBC_STRERROR_R, 1, [we have a glibc-style strerror_r()])
      AC_MSG_RESULT([yes]),
      AC_MSG_NOTICE([cannot determine strerror_r() style: edit lib/config.h manually!])
    ) dnl while cross-compiling
    )

    if test -z "$GLIBC_STRERROR_R"; then

      AC_MSG_CHECKING([for a POSIX strerror_r API])
      AC_TRY_RUN([
#include <string.h>
#include <errno.h>
int
main () {
  char buffer[1024]; /* big enough to play with */
  int error =
    strerror_r(EACCES, buffer, sizeof(buffer));
    /* This should've returned zero, and written an error string in the
       buffer.*/
    if(!buffer[0] || error)
      return 99;
    return 0;
}
],
      AC_DEFINE(HAVE_POSIX_STRERROR_R, 1, [we have a POSIX-style strerror_r()])
      AC_MSG_RESULT([yes]),
      AC_MSG_RESULT([no]) ,
      dnl cross-compiling!
      AC_MSG_NOTICE([cannot determine strerror_r() style: edit lib/config.h manually!])
    )

    fi dnl if not using glibc API

  fi dnl we have a strerror_r

])

AC_DEFUN([CURL_CHECK_INET_NTOA_R],
[
  dnl determine if function definition for inet_ntoa_r exists.
  AC_CHECK_FUNCS(inet_ntoa_r,[
    AC_MSG_CHECKING(whether inet_ntoa_r is declared)
    AC_EGREP_CPP(inet_ntoa_r,[
#include <arpa/inet.h>],[
      AC_DEFINE(HAVE_INET_NTOA_R_DECL, 1, [inet_ntoa_r() is declared])
      AC_MSG_RESULT(yes)],[
      AC_MSG_RESULT(no)
      AC_MSG_CHECKING(whether inet_ntoa_r with -D_REENTRANT is declared)
      AC_EGREP_CPP(inet_ntoa_r,[
#define _REENTRANT
#include <arpa/inet.h>],[
	AC_DEFINE(HAVE_INET_NTOA_R_DECL, 1, [inet_ntoa_r() is declared])
	AC_DEFINE(NEED_REENTRANT, 1, [need REENTRANT defined])
	AC_MSG_RESULT(yes)],
	AC_MSG_RESULT(no))])])
])

AC_DEFUN([CURL_CHECK_GETHOSTBYADDR_R],
[
  dnl check for number of arguments to gethostbyaddr_r. it might take
  dnl either 5, 7, or 8 arguments.
  AC_CHECK_FUNCS(gethostbyaddr_r,[
    AC_MSG_CHECKING(if gethostbyaddr_r takes 5 arguments)
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <netdb.h>],[
char * address;
int length;
int type;
struct hostent h;
struct hostent_data hdata;
int rc;
rc = gethostbyaddr_r(address, length, type, &h, &hdata);],[
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_GETHOSTBYADDR_R_5, 1, [gethostbyaddr_r() takes 5 args])
      ac_cv_gethostbyaddr_args=5],[
      AC_MSG_RESULT(no)
      AC_MSG_CHECKING(if gethostbyaddr_r with -D_REENTRANT takes 5 arguments)
      AC_TRY_COMPILE([
#define _REENTRANT
#include <sys/types.h>
#include <netdb.h>],[
char * address;
int length;
int type;
struct hostent h;
struct hostent_data hdata;
int rc;
rc = gethostbyaddr_r(address, length, type, &h, &hdata);],[
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_GETHOSTBYADDR_R_5, 1, [gethostbyaddr_r() takes 5 args])
	AC_DEFINE(NEED_REENTRANT, 1, [need REENTRANT])
	ac_cv_gethostbyaddr_args=5],[
	AC_MSG_RESULT(no)
	AC_MSG_CHECKING(if gethostbyaddr_r takes 7 arguments)
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <netdb.h>],[
char * address;
int length;
int type;
struct hostent h;
char buffer[8192];
int h_errnop;
struct hostent * hp;

hp = gethostbyaddr_r(address, length, type, &h,
                     buffer, 8192, &h_errnop);],[
	  AC_MSG_RESULT(yes)
	  AC_DEFINE(HAVE_GETHOSTBYADDR_R_7, 1, [gethostbyaddr_r() takes 7 args] )
	  ac_cv_gethostbyaddr_args=7],[
	  AC_MSG_RESULT(no)
	  AC_MSG_CHECKING(if gethostbyaddr_r takes 8 arguments)
	  AC_TRY_COMPILE([
#include <sys/types.h>
#include <netdb.h>],[
char * address;
int length;
int type;
struct hostent h;
char buffer[8192];
int h_errnop;
struct hostent * hp;
int rc;

rc = gethostbyaddr_r(address, length, type, &h,
                     buffer, 8192, &hp, &h_errnop);],[
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(HAVE_GETHOSTBYADDR_R_8, 1, [gethostbyaddr_r() takes 8 args])
	    ac_cv_gethostbyaddr_args=8],[
	    AC_MSG_RESULT(no)
	    have_missing_r_funcs="$have_missing_r_funcs gethostbyaddr_r"])])])])])
])

AC_DEFUN([CURL_CHECK_GETHOSTBYNAME_R],
[
  dnl check for number of arguments to gethostbyname_r. it might take
  dnl either 3, 5, or 6 arguments.
  AC_CHECK_FUNCS(gethostbyname_r,[
    AC_MSG_CHECKING([if gethostbyname_r takes 3 arguments])
    AC_TRY_COMPILE([
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#undef NULL
#define NULL (void *)0

int
gethostbyname_r(const char *, struct hostent *, struct hostent_data *);],[
struct hostent_data data;
gethostbyname_r(NULL, NULL, NULL);],[
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_GETHOSTBYNAME_R_3, 1, [gethostbyname_r() takes 3 args])
      ac_cv_gethostbyname_args=3],[
      AC_MSG_RESULT(no)
      AC_MSG_CHECKING([if gethostbyname_r with -D_REENTRANT takes 3 arguments])
      AC_TRY_COMPILE([
#define _REENTRANT

#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#undef NULL
#define NULL (void *)0

int
gethostbyname_r(const char *,struct hostent *, struct hostent_data *);],[
struct hostent_data data;
gethostbyname_r(NULL, NULL, NULL);],[
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_GETHOSTBYNAME_R_3, 1, [gethostbyname_r() takes 3 args])
	AC_DEFINE(NEED_REENTRANT, 1, [needs REENTRANT])
	ac_cv_gethostbyname_args=3],[
	AC_MSG_RESULT(no)
	AC_MSG_CHECKING([if gethostbyname_r takes 5 arguments])
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <netdb.h>
#undef NULL
#define NULL (void *)0

struct hostent *
gethostbyname_r(const char *, struct hostent *, char *, int, int *);],[
gethostbyname_r(NULL, NULL, NULL, 0, NULL);],[
	  AC_MSG_RESULT(yes)
	  AC_DEFINE(HAVE_GETHOSTBYNAME_R_5, 1, [gethostbyname_r() takes 5 args])
          ac_cv_gethostbyname_args=5],[
	  AC_MSG_RESULT(no)
	  AC_MSG_CHECKING([if gethostbyname_r takes 6 arguments])
	  AC_TRY_COMPILE([
#include <sys/types.h>
#include <netdb.h>
#undef NULL
#define NULL (void *)0

int
gethostbyname_r(const char *, struct hostent *, char *, size_t,
struct hostent **, int *);],[
gethostbyname_r(NULL, NULL, NULL, 0, NULL, NULL);],[
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(HAVE_GETHOSTBYNAME_R_6, 1, [gethostbyname_r() takes 6 args])
            ac_cv_gethostbyname_args=6],[
	    AC_MSG_RESULT(no)
	    have_missing_r_funcs="$have_missing_r_funcs gethostbyname_r"],
	    [ac_cv_gethostbyname_args=0])],
	  [ac_cv_gethostbyname_args=0])],
	[ac_cv_gethostbyname_args=0])],
      [ac_cv_gethostbyname_args=0])])

if test "$ac_cv_func_gethostbyname_r" = "yes"; then
  if test "$ac_cv_gethostbyname_args" = "0"; then
    dnl there's a gethostbyname_r() function, but we don't know how
    dnl many arguments it wants!
    AC_MSG_ERROR([couldn't figure out how to use gethostbyname_r()])
  fi
fi
])



AC_DEFUN([AX_PATH_LIB_CURL], [dnl
AC_MSG_HEADER([Configuring for LibCurl Now])
AX_CONFIG_LIB_CURL
])

AC_DEFUN([AX_PATH_LIB_CURL_UNUSED], [dnl
AC_MSG_CHECKING([lib curl])
AC_ARG_WITH(curl,
[  --with-curl[[=prefix]]    compile libcurl part ],,
     with_curl="yes")
if test ".$with_curl" = ".no" ; then
  AC_MSG_RESULT([disabled])
  m4_ifval($2,$2)
  AX_CONFIG_LIB_CURL
else
  AC_MSG_RESULT([(testing)])
  my_cv_curl_vers=NONE
  dnl check is the plain-text version of the required version
  check="7.10.3"
  dnl check_hex must be UPPERCASE if any hex letters are present
  check_hex="070A03"
 
  AC_MSG_CHECKING([for curl >= $check])
 
  if eval curl-config --version 2>/dev/null >/dev/null; then
   	ver=`curl-config --version | sed -e "s/libcurl //g"`
   	hex_ver=`curl-config --vernum | tr 'a-f' 'A-F'`
   	ok=`echo "ibase=16; if($hex_ver>=$check_hex) $hex_ver else 0" | bc`
 
   	if test x$ok != x0; then
     		my_cv_curl_vers="$ver"
     		AC_MSG_RESULT([$my_cv_curl_vers])
		CURL_LIBS=`curl-config --libs`
		if test -d "/usr/include/curl"; then
			CURL_CFLAGS="-I/usr/include/curl"
		else
			CURL_CFLAGS=`curl-config --cflags`
		fi
	     	AM_CONDITIONAL(BUILD_CURL, false)
 	else
     		AC_MSG_RESULT(FAILED)
     		AX_CONFIG_LIB_CURL
   	fi
  else
   	AC_MSG_RESULT(FAILED)
	AX_CONFIG_LIB_CURL
  fi
fi 
AC_SUBST([CURL_LIBS])
AC_SUBST([CURL_CFLAGS])
])

AC_DEFUN([AX_CONFIG_LIB_CURL],[dnl
AC_MSG_CHECKING([How to build a local curl])
dnl CURL options
AC_SYS_LARGEFILE
CURL_CHECK_FUNC_RECV
CURL_CHECK_FUNC_SEND
CURL_CHECK_MSG_NOSIGNAL
TYPE_SIG_ATOMIC_T
CURL_CHECK_HEADER_MALLOC
CURL_CHECK_TYPE_SOCKLEN_T
CURL_CHECK_FUNC_GETNAMEINFO
TYPE_SOCKADDR_STORAGE
CURL_CHECK_NI_WITHSCOPEID
CURL_CHECK_STRUCT_TIMEVAL
CURL_CHECK_NONBLOCKING_SOCKET
TYPE_IN_ADDR_T
CURL_CHECK_WORKING_RESOLVER
CURL_CHECK_WORKING_GETADDRINFO
CURL_CHECK_LOCALTIME_R
CURL_CHECK_STRERROR_R
CURL_CHECK_INET_NTOA_R
CURL_CHECK_GETHOSTBYADDR_R
CURL_CHECK_GETHOSTBYNAME_R
CURL_CHECK_SSL_ENABLE
CURL_CHECK_ZIP_ENABLE
CURL_CHECK_IPV6_ENABLE

AC_MSG_CHECKING([if we need -no-undefined])
case $host in
  *-*-cygwin | *-*-mingw* | *-*-pw32*)
    need_no_undefined=yes
    ;;
  *)
    need_no_undefined=no
    ;;
esac
AC_MSG_RESULT($need_no_undefined)
dnl AM_CONDITIONAL(NO_UNDEFINED, test x$need_no_undefined = xyes)

AC_MSG_CHECKING([if we need -mimpure-text])
case $host in
  *-*-solaris2*)
    if test "$GCC" = "yes"; then
      mimpure="yes"
    fi
    ;;
  *)
    mimpure=no
    ;;
esac
AC_MSG_RESULT($mimpure)
dnl AM_CONDITIONAL(MIMPURE, test x$mimpure = xyes)
CURL_CHECK_NONBLOCKING_SOCKET
AC_DEFINE(DISABLED_THREADSAFE, 1, Set to explicitly specify we don't want to use thread-safe functions in curl)
dnl gethostbyname in the nsl lib?
AC_CHECK_FUNC(gethostbyname, , [ AC_CHECK_LIB(nsl, gethostbyname) ])

if test "$ac_cv_lib_nsl_gethostbyname" != "yes" -a "$ac_cv_func_gethostbyname" != "yes"; then
  dnl gethostbyname in the socket lib?
  AC_CHECK_FUNC(gethostbyname, , [ AC_CHECK_LIB(socket, gethostbyname) ])
fi

dnl At least one system has been identified to require BOTH nsl and
dnl socket libs to link properly.
if test "$ac_cv_lib_nsl_gethostbyname" != "yes" -a "$ac_cv_lib_socket_gethostbyname" != "yes" -a "$ac_cv_func_gethostbyname" != "yes"; then
  AC_MSG_CHECKING([trying both nsl and socket libs])
  my_ac_save_LIBS=$LIBS
  LIBS="-lnsl -lsocket $LIBS"
  AC_TRY_LINK( ,
             [gethostbyname();],
             my_ac_link_result=success,
             my_ac_link_result=failure )

  if test "$my_ac_link_result" = "failure"; then
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([couldn't find libraries for gethostbyname()])
    dnl restore LIBS
    LIBS=$my_ac_save_LIBS
  else
    AC_MSG_RESULT([yes])
  fi
fi

dnl resolve lib?
AC_CHECK_FUNC(strcasecmp, , [ AC_CHECK_LIB(resolve, strcasecmp) ])

if test "$ac_cv_lib_resolve_strcasecmp" = "$ac_cv_func_strcasecmp"; then
  AC_CHECK_LIB(resolve, strcasecmp,
              [LIBS="-lresolve $LIBS"],
               ,
               -lnsl)
fi

dnl socket lib?
AC_CHECK_FUNC(connect, , [ AC_CHECK_LIB(socket, connect) ])

AC_MSG_CHECKING([if argv can be written to])
AC_CACHE_VAL(curl_cv_writable_argv, [
AC_RUN_IFELSE([[
int main(int argc, char ** argv) {
	argv[0][0] = ' ';
	return (argv[0][0] == ' ')?0:1;
}
	]],
	curl_cv_writable_argv=yes,
	curl_cv_writable_argv=no,
	curl_cv_writable_argv=cross)
])
case $curl_cv_writable_argv in
yes)
	AC_DEFINE(HAVE_WRITABLE_ARGV, 1, [Define this symbol if your OS supports changing the contents of argv])
	AC_MSG_RESULT(yes)
	;;
no)
	AC_MSG_RESULT(no)
	;;
*)
        AC_MSG_RESULT(no)
        AC_MSG_WARN([the previous check could not be made default was used])
	;;
esac


AC_DEFINE(CURL_DISABLE_LDAP, 1, [to disable LDAP])
AC_SUBST(CURL_DISABLE_LDAP, [1])

CURL_LIBS='${top_srcdir}/'lib/curl/libcurl.la
CURL_CFLAGS='-I${top_srcdir}/'lib/curl
AC_SUBST([CURL_LIBS])
AC_SUBST([CURL_CFLAGS])
AM_CONDITIONAL(BUILD_CURL, true)
])
dnl TYPE_SIG_ATOMIC_T
dnl -------------------------------------------------
dnl Check if the sig_atomic_t type is available, and
dnl verify if it is already defined as volatile.

AC_DEFUN([TYPE_SIG_ATOMIC_T], [
  AC_CHECK_HEADERS(signal.h)
  AC_CHECK_TYPE([sig_atomic_t],[
    AC_DEFINE(HAVE_SIG_ATOMIC_T, 1,
      [Define to 1 if sig_atomic_t is an available typedef.])
  ], ,[
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
  ])
  case "$ac_cv_type_sig_atomic_t" in
    yes)
      #
      AC_MSG_CHECKING([if sig_atomic_t is already defined as volatile])
      AC_TRY_LINK([
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
        ],[
          static volatile sig_atomic_t dummy = 0;
        ],[ 
          AC_MSG_RESULT([no])
          ac_cv_sig_atomic_t_volatile="no"
        ],[
          AC_MSG_RESULT([yes])
          ac_cv_sig_atomic_t_volatile="yes"
      ])
      #
      if test "$ac_cv_sig_atomic_t_volatile" = "yes"; then
        AC_DEFINE(HAVE_SIG_ATOMIC_T_VOLATILE, 1,
          [Define to 1 if sig_atomic_t is already defined as volatile.])
      fi
      ;;
  esac
]) # AC_DEFUN
