pkglib_LTLIBRARIES	= statserv.la
statserv_la_SOURCES	= ss_help.c \
			stats.c stats.h statserv.c statserv.h \
			htmlstats.c htmlstats.h \
			tld.c tld.h channel.c channel.h \
			user.c user.h network.c network.h \
			server.c server.h version.c version.h \
			geoip/GeoIPCity.c geoip/GeoIPCity.h \
			geoip/GeoIPBitReader.c geoip/GeoIPBitReader.h \
			geoip/GeoIP.c geoip/GeoIP.h
statserv_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/modules/statserv/geoip -DGEOIPDATADIR=\"$(datadir)\" \
			-DSYSCONFDIR=\"$(sysconfdir)\"
dist_data_DATA		= GeoIP.dat html/index.tpl
include $(top_srcdir)/autotools/rules.mk
