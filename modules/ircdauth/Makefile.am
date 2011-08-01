pkglib_LTLIBRARIES	= ircdauth.la
ircdauth_la_SOURCES	= ircdauth.c 
ircdauth_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
