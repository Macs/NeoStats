pkglib_LTLIBRARIES	= update.la
update_la_SOURCES	= update.c update.h update_help.c
update_la_LDFLAGS	= -module -avoid-version --export-dynamic
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/lib/nxml/
include $(top_srcdir)/autotools/rules.mk
