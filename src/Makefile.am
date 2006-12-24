bin_PROGRAMS = gprayer

gprayer_SOURCES = defines.h \
                  main.h    \
		  prefs.h    \
		  main.c

gprayer_CPPFLAGS = \
	-I$(top_srcdir)					\
	-DG_LOG_DOMAIN=\"Gprayer\"			\
	-DDATADIR=\""$(pkgdatadir)"\"			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DGPRAYER_GLADEDIR=\"$(datadir)/$(PACKAGE)\" \
	-DGPRAYER_PREFDIR=\""/apps/gprayer"\" \
	-DGPRAYER_ATHAN_DIR=\"$(datadir)/$(PACKAGE)\" \
	-DGPRAYER_PIXMAPSDIR=\"$(datadir)/$(PACKAGE)/pixmaps\" \
	$(DISABLE_DEPRECATED)				\
	$(AM_CPPFLAGS)

gprayer_CFLAGS = \
	   $(GPRAYER_CFLAGS)	\
	   $(WARN_CFLAGS)		\
	   $(AM_CFLAGS)

gprayer_LDFLAGS = \
	$(AM_LDFLAGS)

gprayer_LDADD = @ITL_LIBS@ \
	$(GPRAYER_LIBS)	\
 	$(INTLLIBS)

if USE_NOTIFY
gprayer_LDADD += $(NOTIFY_LIBS)
gprayer_CPPFLAGS += $(NOTIFY_CFLAGS)
endif


