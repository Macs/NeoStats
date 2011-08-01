pkglib_LTLIBRARIES	= limitserv.la
limitserv_la_SOURCES	= main.c limitserv.h help.c
limitserv_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
