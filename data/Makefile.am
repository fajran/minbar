EXTRA_DIST = athan.ogg minbar.glade minbar.schemas

@INTLTOOL_SCHEMAS_RULE@
schemadir = $(GCONF_SCHEMA_FILE_DIR)
schemasdir = $(sysconfdir)/gconf/schemas
schemas_in_files = minbar.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)



install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/$(PACKAGE)
	$(INSTALL_DATA) athan.ogg minbar.glade $(DESTDIR)$(datadir)/$(PACKAGE)
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(schemas_DATA)

## End Makefile.am
