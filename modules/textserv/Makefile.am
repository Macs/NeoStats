pkglib_LTLIBRARIES	= textserv.la
textserv_la_SOURCES	= main.c textserv.h help.c
textserv_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@

dist_data_DATA		= barmaid.tsdb icecream.tsdb loveserv.tsdb moraleserv.tsdb waitress.tsdb
include $(top_srcdir)/autotools/rules.mk
