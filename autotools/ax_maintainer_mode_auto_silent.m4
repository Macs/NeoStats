dnl @* AX_MAINTAINER_MODE_AUTO_SILENT
dnl
dnl Set autotools to error/sleep settings so that they are not run when
dnl being errornously triggered. Likewise make libtool-silent when 
dnl libtool has been used.
dnl
dnl I am using the macro quite a lot since some automake versions had the
dnl tendency to try to rerun some autotools on a mere make even when not
dnl quite in --maintainer-mode. That is very annoying. Likewise, a user
dnl who installs from source does not want to see doubled compiler messages.
dnl
dnl I did not put an AC-REQUIRE(MAINTAINER_MODE) in here - should I?
dnl
dnl @: guidod@gmx.de
dnl @$Id: ax_maintainer_mode_auto_silent.m4,v 1.1 2003/10/19 00:12:45 guidod Exp $

AC_DEFUN([AX_MAINTAINER_MODE_AUTO_SILENT],
[AC_MSG_CHECKING(silent building of source files)
   if test ".$LIBTOOL" != "." ; then
      LIBTOOL="$LIBTOOL --quiet"
      AC_MSG_RESULT([Enabled])
   else
      AC_MSG_RESULT([Disabled])
   fi
])
