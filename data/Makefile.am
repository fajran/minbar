dist_pkgdata_DATA = Compass.svg athan.ogg minbar.glade Locations.xml minbar-applet.server minbar-applet.xml 

serverdir = $(libdir)/bonobo/servers
server_in_files = minbar-applet.server.in
server_DATA = $(server_in_files:.server.in=.server)


if USE_RSVG
dist_pkgdata_DATA += Compass.svg
endif



if USE_GCONF
@INTLTOOL_SCHEMAS_RULE@


schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = minbar.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

minbar.schemas.in: $(srcdir)/minbar.schemas.in.in
	 sed     -e 's,[@]datadir[@],$(datadir),g' \
		-e 's,[@]PACKAGE[@],$(PACKAGE),g' \
	 <$(srcdir)/minbar.schemas.in.in >minbar.schemas.in

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule  minbar.schemas 2>&1 > /dev/null
	gconftool-2 --shutdown

uninstall-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-uninstall-rule minbar.schemas

endif

$(server_DATA): $(server_in_files)
	sed -e "s|\@BINDIR\@|$(bindir)|" $< > $@

EXTRA_DIST =						\
	minbar.schemas.in.in				\
	$(NULL)

CLEANFILES = \
	$(schemas_DATA)					\
	$(schemas_in_files)				\
	minbar.schemas.in				\
	$(server_DATA) \
	$(NULL)

xmldir = $(datadir)/minbar-applet
xml_DATA = minbar-applet.xml

## End Makefile.am
