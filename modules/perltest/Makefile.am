if DO_PERL
pkglib_LTLIBRARIES	= perlexttest.la
perlexttest_la_SOURCES	= main.c 
perlexttest_la_LDFLAGS	= -module -avoid-version
endif

AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@ @PERL_CFLAGS@
EXTRA_DIST		= extension.ple test.pl NamedVarsInspector.pl


include $(top_srcdir)/autotools/rules.mk
