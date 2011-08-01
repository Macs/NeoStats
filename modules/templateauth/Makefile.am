pkglib_LTLIBRARIES	= templateauth.la
templateauth_la_SOURCES	= main.c 
templateauth_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
