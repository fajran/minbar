EXTRA_DIST = gprayer.png kaabatrayicon.png quran.png

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	for pixmap in *.png; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	    fi \
	done

dist-hook:
	mkdir $(distdir)/pixmaps; \
	for pixmap in *.png; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	done

## End Makefile.am
