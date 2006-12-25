EXTRA_DIST = athan.ogg minbar.glade minbar.schemas

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = minbar.schemas

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/$(PACKAGE)
	$(INSTALL_DATA) athan.ogg minbar.glade $(DESTDIR)$(datadir)/$(PACKAGE)
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(schema_DATA)

## End Makefile.am
