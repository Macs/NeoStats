pkglib_LTLIBRARIES	= operlog.la
operlog_la_SOURCES	= main.c help.c operlog.h
operlog_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
