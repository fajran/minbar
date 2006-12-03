## Makefile.am - process this file with automake
## Anthony Ventimiglia
## Copyright 2002 GNU GPL

bin_PROGRAMS = gprayer

gprayer_SOURCES = defines.h \
                  main.c    \
		  main.h    \
		  prefs.h

gprayer_LDADD = @PACKAGE_LIBS@ @ITL_LIBS@

INCLUDES = -DGPRAYER_GLADEDIR=\"$(datadir)/$(PACKAGE)\" \
	   -DGPRAYER_PREFDIR=\""/apps/prayertimes"\" \
	   -DGPRAYER_ATHAN_DIR=\"$(datadir)/$(PACKAGE)\" \
           @PACKAGE_CFLAGS@

EXTRA_DIST = gprayer.glade athan.ogg

##schemadir   = @GCONF_SCHEMA_FILE_DIR@
##schema_DATA = gprayer.schemas
#
install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/$(PACKAGE)
	$(INSTALL_DATA) gprayer.glade $(DESTDIR)$(datadir)/$(PACKAGE)
	$(INSTALL_DATA) athan.ogg $(DESTDIR)$(datadir)/$(PACKAGE)
##	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
##	gconftool-2 --makefile-install-rule $(srcdir)/$(schema_DATA)

## End Makefile.am
