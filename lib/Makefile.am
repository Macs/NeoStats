SUBDIRS		= pcre curl event nxml $(BLD_LIBEXEC)
DIST_SUBDIRS	= pcre curl event nxml execinfo
lib_LTLIBRARIES = libneostats.la
libneostats_la_SOURCES =
libneostats_la_LIBADD = pcre/libpcre.la nxml/libmrss.la curl/libcurl.la $(LNK_LIBEXEC)
libneostats_la_LDFLAGS =-version-info 1:0:0

include $(top_srcdir)/autotools/rules.mk
