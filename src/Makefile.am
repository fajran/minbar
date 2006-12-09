## Process this file with automake to produce Makefile.in

bin_PROGRAMS = gprayer

gprayer_SOURCES = defines.h \
                  main.c    \
		  main.h    \
		  prefs.h

gprayer_LDADD = @PACKAGE_LIBS@ @ITL_LIBS@

INCLUDES = -DGPRAYER_GLADEDIR=\"$(datadir)/$(PACKAGE)\" \
	   -DGPRAYER_PREFDIR=\""/apps/gprayer"\" \
	   -DGPRAYER_ATHAN_DIR=\"$(datadir)/$(PACKAGE)\" \
	   -DGPRAYER_PIXMAPSDIR=\"$(datadir)/$(PACKAGE)\" \
	   @PACKAGE_CFLAGS@

if USE_NOTIFY
gprayer_LDADD += $(NOTIFY_LIBS)
INCLUDES += $(NOTIFY_CFLAGS)
endif

EXTRA_DIST = gprayer.glade athan.ogg quran.png kaabatrayicon.png gprayer.schemas

#schemadir   = @GCONF_SCHEMA_FILE_DIR@
#schema_DATA = gprayer.schemas
#
#install-data-local:
#	@$(NORMAL_INSTALL)
#	$(mkinstalldirs) $(DESTDIR)$(datadir)/$(PACKAGE)
#	$(INSTALL_DATA) athan.ogg gprayer.glade quran.png kaabatrayicon.png $(DESTDIR)$(datadir)/$(PACKAGE)
#	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(srcdir)/$(schema_DATA)

## End Makefile.am
