pkglib_LTLIBRARIES	= gdbm.la 
gdbm_la_SOURCES = gdbm.c \
	bucket.c falloc.c findkey.c \
	gdbmclose.c gdbmdelete.c gdbmerrno.c gdbmexists.c gdbmfdesc.c \
	gdbmfetch.c gdbmopen.c gdbmreorg.c gdbmseq.c gdbmsetopt.c \
	gdbmstore.c gdbmsync.c global.c hash.c update.c version.c

noinst_HEADERS	= extern.h gdbmerrno.h systems.h gdbmconst.h getopt.h gdbmdefs.h \
		  proto.h

AM_LDFLAGS	= -module -avoid-version
AM_CFLAGS	= -I$(top_srcdir)/include -I$(top_srcdir)/lib/adns/ @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
