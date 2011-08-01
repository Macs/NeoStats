SUBDIRS		= gdbm $(BLD_BDB)
DIST_SUBDIRS	= gdbm bdb
bin_PROGRAMS 	= convertdb 

convertdb_SOURCES = convertdb.c ../../src/dl.c ../../src/hash.c ../../src/list.c ../../src/ircstring.c ../../src/nsmemory.c ../../src/osfile.c ../../src/support.c
convertdb_LDADD   = $(top_builddir)/lib/libneostats.la
AM_CFLAGS         = -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@

include $(top_srcdir)/autotools/rules.mk

