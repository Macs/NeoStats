#
# $Id: Makefile.am,v 1.72 2003/10/24 21:26:05 bagder Exp $
#

AUTOMAKE_OPTIONS = foreign nostdinc


noinst_LTLIBRARIES = libcurl.la

# we use srcdir/include for the static global include files
# we use builddir/lib for the generated lib/config.h file to get found
# we use srcdir/lib for the lib-private header files
INCLUDES = -I$(top_srcdir)/include 

libcurl_la_LDFLAGS = $(VERSION)

libcurl_la_SOURCES = file.c timeval.c base64.c hostip.c progress.c formdata.c	\
  cookie.c http.c sendf.c ftp.c url.c dict.c if2ip.c speedcheck.c	\
  ldap.c ssluse.c version.c getenv.c escape.c mprintf.c telnet.c	\
  netrc.c getinfo.c transfer.c strequal.c easy.c security.c krb4.c	\
  memdebug.c http_chunks.c strtok.c connect.c llist.c hash.c multi.c	\
  content_encoding.c share.c http_digest.c md5.c http_negotiate.c	\
  http_ntlm.c inet_pton.c strtoofft.c strerror.c hostares.c hostasyn.c	\
  hostip4.c hostip6.c hostsyn.c hostthre.c inet_ntop.c parsedate.c	\
  select.c gtls.c sslgen.c tftp.c splay.c strdup.c socks.c ssh.c qssl.c

noinst_HEADERS = arpa_telnet.h netrc.h file.h timeval.h base64.h hostip.h	\
  progress.h formdata.h cookie.h http.h sendf.h ftp.h url.h dict.h	\
  if2ip.h speedcheck.h urldata.h ldap.h ssluse.h escape.h telnet.h	\
  getinfo.h strequal.h krb4.h memdebug.h inet_ntoa_r.h \
  http_chunks.h strtok.h connect.h llist.h hash.h content_encoding.h 	\
  share.h md5.h http_digest.h http_negotiate.h http_ntlm.h ca-bundle.h	\
  inet_pton.h strtoofft.h strerror.h inet_ntop.h curlx.h memory.h	\
  setup.h transfer.h select.h easyif.h multiif.h parsedate.h sslgen.h   \
  gtls.h tftp.h sockaddr.h splay.h strdup.h setup_once.h socks.h ssh.h  \
  qssl.h nssg.h curl_ldap.h

include_HEADERS = curl.h curlver.h easy.h multi.h mprintf.h

BUILT_SOURCES = $(srcdir)/ca-bundle.h

$(srcdir)/ca-bundle.h: Makefile.in Makefile
	@if test -f $@; then \
	  chmod 0644 $@; \
	fi
	@echo "/* The file is generated automaticly */" > $@
	@echo '#undef CURL_CA_BUNDLE /* unknown */' >> $@

include $(top_srcdir)/autotools/rules.mk
