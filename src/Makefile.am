bin_PROGRAMS = neostats

neostats_SOURCES = auth.c bans.c bots.c channels.c commands.c confuse.c confuselexer.c nsdba.c \
		conf.c dl.c dns.c exclude.c hash.c ircstring.c \
		ircprotocol.c ircsend.c ircrecv.c \
		lang.c list.c log.c main.c match.c misc.c modules.c nsevents.c \
		helpstrings.c servers.c services.c signals.c sock.c support.c \
		timer.c transfer.c users.c modes.c ctcp.c dcc.c nsmemory.c \
		base64.c numerics.c settings.c botinfo.c \
		adnscheck.c adnsevent.c adnsgeneral.c adnsparse.c adnsquery.c \
		adnsreply.c adnssetup.c adnstransmit.c adnstypes.c \
		oscalls.c ossocket.c osfile.c updates.c MiniMessage.c MiniMessageGateway.c namedvars.c 

if DO_PERL
neostats_SOURCES += perl.c neostats.pm.h NV.c
BUILT_SOURCES 	= NV.c
CLEANFILES 	= NV.c
else
neostats_SOURCES +=
endif

AM_CFLAGS = -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@ -DNEOSTATSCORE @PERL_CFLAGS@ \
		-I$(top_srcdir)/lib/event -I$(top_srcdir)/lib/nxml 
		
neostats_LDADD = @LIBDB@ @PERL_LDFLAGS@ $(top_builddir)/lib/libneostats.la $(top_builddir)/lib/event/libevent.la 

EXTRA_DIST	= typemap NV.xs

noinst_HEADERS          =       confuselexer.l NeoStats.pm

neostats.pm.h: NeoStats.pm 	 
		$(top_builddir)/tools/generate_header

.xs.c:
	xsubpp -prototypes -typemap $(srcdir)/typemap $*.xs > $*.c

include $(top_srcdir)/autotools/rules.mk 
