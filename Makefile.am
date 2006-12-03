## Makefile.am - process this file with automake
## Anthony Ventimiglia
## Copyright 2002 GNU GPL

bin_PROGRAMS = gprayer

gprayer_SOURCES = defines.h \
                  main.c    \
		  main.h    \
		  prefs.h

gprayer_LDADD = @PACKAGE_LIBS@ @ITL_LIBS@

INCLUDES = -DGPRAYER_GLADEDIR=\"./\" \
	   -DGPRAYER_PREFDIR=\""/apps/prayertimes"\" \
	   -DGPRAYER_ATHAN_DIR=\"`pwd`/\" \
           @PACKAGE_CFLAGS@

EXTRA_DIST = gprayer.xml

##schemadir   = @GCONF_SCHEMA_FILE_DIR@
##schema_DATA = gprayer.schemas
#
##install-data-local:
##	@$(NORMAL_INSTALL)
##	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
##	gconftool-2 --makefile-install-rule $(srcdir)/$(schema_DATA)

## End Makefile.am
