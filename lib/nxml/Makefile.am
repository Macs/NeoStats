noinst_LTLIBRARIES	= libmrss.la
libmrss_la_SOURCES	= nxml_download.c nxml_easy.c nxml_free.c nxml_namespace.c nxml_string.c nxml_utf.c \
       			  nxml_edit.c nxml_error.c nxml_init.c nxml_parser.c nxml_tools.c nxml_write.c mrss_download.c mrss_edit.c \
       			  mrss_free.c mrss_generic.c mrss_parser.c mrss_write.c mrss_options.c mrss_search.c
libmrss_la_CPPFLAGS	= -DNXML_INTERNAL
include_HEADERS 	= nxml.h mrss.h
noinst_HEADERS		= nxml_internal.h mrss_internal.h
INCLUDES 		= -I$(top_srcdir)/lib/curl/

EXTRA_DIST = 

include $(top_srcdir)/autotools/rules.mk


