EXTRA_DIST = athan.ogg gprayer.glade gprayer.schemas

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gprayer.schemas

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/$(PACKAGE)
	$(INSTALL_DATA) athan.ogg gprayer.glade $(DESTDIR)$(datadir)/$(PACKAGE)
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(schema_DATA)

## End Makefile.am
