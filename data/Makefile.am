dist_pkgdata_DATA = athan.ogg minbar.glade Locations.xml

if USE_GCONF
@INTLTOOL_SCHEMAS_RULE@


schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = minbar.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

minbar.schemas.in: $(srcdir)/minbar.schemas.in.in
	 sed     -e 's,[@]datarootdir[@],$(datadir),g' \
		-e 's,[@]PACKAGE[@],$(PACKAGE),g' \
	 <$(srcdir)/minbar.schemas.in.in >minbar.schemas.in

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule  minbar.schemas 2>&1 > /dev/null
	gconftool-2 --shutdown

uninstall-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-uninstall-rule minbar.schemas

endif
EXTRA_DIST =						\
	$(schemas_in_files)				\
	$(schemas_DATA)					\
	minbar.schemas.in.in				\
	$(NULL)




## End Makefile.am
