pkglib_LTLIBRARIES	= hostserv.la
hostserv_la_SOURCES	= hostserv.c hostserv.h hs_help.c
hostserv_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk