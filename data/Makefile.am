dist_pkgdata_DATA = athan.ogg minbar.glade Locations.xml

if USE_GCONF
@INTLTOOL_SCHEMAS_RULE@
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = minbar.schemas

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule  minbar.schemas 2>&1 > /dev/null
	gconftool-2 --shutdown # TODO: why this?

uninstall-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-uninstall-rule minbar.schemas

endif

## End Makefile.am
