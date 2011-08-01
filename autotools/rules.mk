LINK = 	$(LIBTOOL) --tag=CC --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
        $(AM_LDFLAGS) $(LDFLAGS) -o $@

.c.o:
@USECCDV@		@echo "Building $@"
@am__fastdepCC_TRUE@	@if $(COMPILE) -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
@am__fastdepCC_TRUE@	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
@am__fastdepCC_TRUE@	then mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
@am__fastdepCC_TRUE@	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
@am__fastdepCC_TRUE@	fi
@AMDEP_TRUE@@am__fastdepCC_FALSE@	@source='$<' object='$@' libtool=no @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	depfile='$(DEPDIR)/$*.Po' tmpdepfile='$(DEPDIR)/$*.TPo' @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	$(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCC_FALSE@	$(COMPILE) -c `test -f '$<' || echo '$(srcdir)/'`$<

.c.obj:
@USECCDV@		@echo "Building $@"
@am__fastdepCC_TRUE@	@if $(LTCOMPILE) -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
@am__fastdepCC_TRUE@	  -c -o $@ `if test -f '$<'; then $(CYGPATH_W) '$<'; else $(CYGPATH_W) '$(srcdir)/$<'; fi`; \
@am__fastdepCC_TRUE@	then mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
@am__fastdepCC_TRUE@	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
@am__fastdepCC_TRUE@	fi
@AMDEP_TRUE@@am__fastdepCC_FALSE@	@source='$<' object='$@' libtool=no @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	depfile='$(DEPDIR)/$*.Po' tmpdepfile='$(DEPDIR)/$*.TPo' @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	$(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCC_FALSE@	$(LTCOMPILE) -c `if test -f '$<'; then $(CYGPATH_W) '$<'; else $(CYGPATH_W) '$(srcdir)/$<'; fi`

.c.lo:
@USECCDV@		@echo "Building $@"
@am__fastdepCC_TRUE@	@if $(LTCOMPILE) -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
@am__fastdepCC_TRUE@	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
@am__fastdepCC_TRUE@	then mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Plo"; \
@am__fastdepCC_TRUE@	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
@am__fastdepCC_TRUE@	fi
@AMDEP_TRUE@@am__fastdepCC_FALSE@	@source='$<' object='$@' libtool=yes @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	depfile='$(DEPDIR)/$*.Plo' tmpdepfile='$(DEPDIR)/$*.TPlo' @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCC_FALSE@	$(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCC_FALSE@	$(LTCOMPILE) -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<


install-pkglibLTLIBRARIES: $(pkglib_LTLIBRARIES)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkglibdir)
	@list='ls .libs/*.so'; for p in $$list; do \
	  if test -f $$p; then \
	    f="`echo $$p | sed -e 's|^.*/||'`"; \
	    $(CCDV) $(LIBTOOL) --mode=install $(pkglibLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) $$p $(DESTDIR)$(pkglibdir)/$$f; \
       	    if test "x#" != "x@USECCDV@"; then echo "Installing $$f"; fi; \
	  else :; fi; \
	done

install-binSCRIPTS: $(bin_SCRIPTS)
	@$(NORMAL_INSTALL)
	test -z "$(bindir)" || $(mkdir_p) "$(DESTDIR)$(bindir)"
	@list='$(bin_SCRIPTS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  if test -f $$d$$p; then \
	    f=`echo "$$p" | sed 's|^.*/||;$(transform)'`; \
	    $(CCDV) $(binSCRIPT_INSTALL) "$$d$$p" "$(DESTDIR)$(bindir)/$$f"; \
   	if test "x#" != "x@USECCDV@"; then echo "Installing $$f"; fi; \
	  else :; fi; \
	done

install-dist_binSCRIPTS: $(dist_bin_SCRIPTS)
	@$(NORMAL_INSTALL)
	test -z "$(bindir)" || $(mkdir_p) "$(DESTDIR)$(bindir)"
	@list='$(dist_bin_SCRIPTS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  if test -f $$d$$p; then \
	    f=`echo "$$p" | sed 's|^.*/||;$(transform)'`; \
	    $(CCDV) $(dist_binSCRIPT_INSTALL) "$$d$$p" "$(DESTDIR)$(bindir)/$$f"; \
   	if test "x#" != "x@USECCDV@"; then echo "Installing $$f"; fi; \
	  else :; fi; \
	done

install-dist_dataDATA: $(dist_data_DATA)
	@$(NORMAL_INSTALL)
	test -z "$(datadir)" || $(mkdir_p) "$(DESTDIR)$(datadir)"
	@list='$(dist_data_DATA)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f=$(am__strip_dir) \
	  $(CCDV) $(dist_dataDATA_INSTALL) "$$d$$p" "$(DESTDIR)$(datadir)/$$f"; \
   	if test "x#" != "x@USECCDV@"; then echo "Installing $$f"; fi; \
	done

install-dist_docDATA: $(dist_doc_DATA)
	@$(NORMAL_INSTALL)
	test -z "$(docdir)" || $(mkdir_p) "$(DESTDIR)$(docdir)"
	@list='$(dist_doc_DATA)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f=$(am__strip_dir) \
	  $(CCDV) $(dist_docDATA_INSTALL) "$$d$$p" "$(DESTDIR)$(docdir)/$$f"; \
   	if test "x#" != "x@USECCDV@"; then echo "Installing $$f"; fi; \
	done

install-includeHEADERS: $(include_HEADERS)
	@$(NORMAL_INSTALL)
	test -z "$(includedir)" || $(mkdir_p) "$(DESTDIR)$(includedir)"
	@list='$(include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f=$(am__strip_dir) \
	  $(CCDV) $(includeHEADERS_INSTALL) "$$d$$p" "$(DESTDIR)$(includedir)/$$f"; \
   	if test "x#" != "x@USECCDV@"; then echo "Installing $$f"; fi; \
	done

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	test -z "$(bindir)" || $(mkdir_p) "$(DESTDIR)$(bindir)"
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  if test -f $$p \
	     || test -f $$p1 \
	  ; then \
	    f=`echo "$$p1" | sed 's,^.*/,,;$(transform);s/$$/$(EXEEXT)/'`; \
	   $(CCDV) $(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(binPROGRAMS_INSTALL) "$$p" "$(DESTDIR)$(bindir)/$$f" || exit 1; \
   	if test "x#" != "x@USECCDV@"; then echo "Installing $$f"; fi; \
	  else :; fi; \
	done

install-libLTLIBRARIES: $(lib_LTLIBRARIES)
	@$(NORMAL_INSTALL)
	test -z "$(libdir)" || $(mkdir_p) "$(DESTDIR)$(libdir)"
	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    f=$(am__strip_dir) \
	    $(CCDV) $(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$(DESTDIR)$(libdir)/$$f"; \
   	if test "x#" != "x@USECCDV@"; then echo "Installing $$f"; fi; \
	  else :; fi; \
	done

install-neoSCRIPTS: $(neo_SCRIPTS)
	@$(NORMAL_INSTALL)
	test -z "$(neodir)" || $(mkdir_p) "$(DESTDIR)$(neodir)"
	@list='$(neo_SCRIPTS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  if test -f $$d$$p; then \
	    f=`echo "$$p" | sed 's|^.*/||;$(transform)'`; \
	    $(CCDV) $(neoSCRIPT_INSTALL) "$$d$$p" "$(DESTDIR)$(neodir)/$$f"; \
	  else :; fi; \
	done

pkglibdir = $(prefix)/modules
datadir = $(prefix)/data
docdir = $(prefix)/doc

distdir			= @DISTDIRVERSION@
distcleancheck:
		@:
distuninstallcheck:
		@:
