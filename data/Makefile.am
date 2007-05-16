EXTRA_DIST = athan.ogg minbar.glade minbar.schemas

@INTLTOOL_SCHEMAS_RULE@
SCHEMAS_FILE = @PACKAGE_NAME@.schemas
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = minbar.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

# TODO: there should be a uninstall aswell
# TODO: depends on USE_GCONF
install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/$(PACKAGE)
	$(INSTALL_DATA) athan.ogg minbar.glade $(DESTDIR)$(datadir)/$(PACKAGE)
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(SCHEMAS_FILE) 2>&1 > /dev/null
	gconftool-2 --shutdown

## End Makefile.am
