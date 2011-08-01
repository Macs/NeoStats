noinst_LTLIBRARIES	= libpcre.la
noinst_PROGRAMS		= dftables
libpcre_la_SOURCES	= chartables.c get.c pcre.c study.c internal.h pcre.h
dftables_SOURCES	= dftables.c internal.h pcre.h
dftables_DEPENDENCES 	= maketables.c
noinst_HEADERS		= maketables.c

include_HEADERS		= pcre.h

AM_CFLAGS 		= @PCRE_CFLAGS@

chartables.c: dftables
	$(top_builddir)/lib/pcre/dftables > $@

include $(top_srcdir)/autotools/rules.mk
