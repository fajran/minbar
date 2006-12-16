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
	   -DGPRAYER_PIXMAPSDIR=\"$(datadir)/pixmaps/$(PACKAGE)\" \
	   -DLOCALE_DIR=\""$(datadir)/locale"\" \
	   @PACKAGE_CFLAGS@

if USE_NOTIFY
gprayer_LDADD += $(NOTIFY_LIBS)
INCLUDES += $(NOTIFY_CFLAGS)
endif

## End Makefile.am

