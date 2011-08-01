pkglib_LTLIBRARIES		= extauth.la
extauth_la_SOURCES	= extauth.c 
extauth_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk