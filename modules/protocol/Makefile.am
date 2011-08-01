pkglib_LTLIBRARIES	= bahamut14.la bahamut18.la \
						client.la \
						hybrid6.la hybrid7.la \
						ircup10.la asuka.la nefarious.la \
						plexus.la plexus3.la \
						ultimate2.la ultimate3.la \
						unreal31.la unreal32.la inspircd.la
bahamut14_la_SOURCES = bahamut14.c
bahamut18_la_SOURCES = bahamut18.c
client_la_SOURCES = client.c
hybrid7_la_SOURCES = hybrid7.c
hybrid6_la_SOURCES = hybrid6.c
ircup10_la_SOURCES = ircup10.c 
EXTRA_DIST = ircup10base.c
asuka_la_SOURCES = asuka.c
nefarious_la_SOURCES = nefarious.c
plexus_la_SOURCES = plexus.c
plexus3_la_SOURCES = plexus3.c
ultimate2_la_SOURCES = ultimate2.c
ultimate3_la_SOURCES = ultimate3.c
unreal31_la_SOURCES = unreal31.c
unreal32_la_SOURCES = unreal32.c
inspircd_la_SOURCES = inspircd.c
AM_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
