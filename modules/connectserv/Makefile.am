pkglib_LTLIBRARIES	= connectserv.la
connectserv_la_SOURCES	= cs.c cs.h cs_help.c
connectserv_la_LDFLAGS	= -module -avoid-version --export-dynamic
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
