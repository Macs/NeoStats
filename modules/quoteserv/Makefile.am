pkglib_LTLIBRARIES	= quoteserv.la
quoteserv_la_SOURCES	= main.c quoteserv.h help.c
quoteserv_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@
dist_data_DATA		= data/discworld data/redneck
include $(top_srcdir)/autotools/rules.mk
