AUTOMAKE_OPTIONS 	= foreign
SUBDIRS 		= include lib src modules
ACLOCAL_AMFLAGS 	= -I autotools
AM_MAKEFLAGS		= -s
DISTCHECK_CONFIGURE_FLAGS = --cache-file=../config.cache
dist_bin_SCRIPTS 	= makeconf
dist_data_DATA		= neostats.motd
#docdir			= @prefix@/doc
dist_doc_DATA		= doc/README.dev ChangeLog.old doc/CODINGMODULES.txt RELNOTES CREDITS README
neodir			= $(prefix)
neo_SCRIPTS		= neostats
DISTCLEANFILES		= autotools/ccdv

EXTRA_DIST		= autotools/ccdv.c autotools/shtool INSTNOTES

all-local:
		if test ! -f .message; then \
		echo ""; \
		$(top_srcdir)/autotools/shtool echo -e "%BCompilation complete.%b"; \
		echo ""; \
		echo "Run 'make install' (or 'gmake install' on some systems) to install NeoStats."; \
		echo "If you require support, see the README file."; \
		touch .message;\
		fi;


install-data-hook:
		test -z "$(prefix)/logs" || $(CCDV) $(mkdir_p) "$(prefix)/logs";
		echo ""
		$(top_srcdir)/autotools/shtool echo -e "%BInstallation complete.%b"
		echo ""
		echo "Change directory to '$(prefix)' then run './neostats'"
		echo "If you require support, see the README file."
		rm -f .message

include $(top_srcdir)/autotools/rules.mk
