bin_PROGRAMS = minbar

minbar_SOURCES = defines.h \
                  main.h    \
		  prefs.h    \
		  main.c

minbar_CPPFLAGS = \
	-I$(top_srcdir)					\
	-DG_LOG_DOMAIN=\"Minbar\"			\
	-DDATADIR=\""$(pkgdatadir)"\"			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DMINBAR_GLADEDIR=\"$(datadir)/$(PACKAGE)\" \
	-DMINBAR_PREFDIR=\""/apps/minbar"\" \
	-DMINBAR_ATHAN_DIR=\"$(datadir)/$(PACKAGE)\" \
	-DMINBAR_PIXMAPSDIR=\"$(datadir)/pixmaps\" \
	$(DISABLE_DEPRECATED)				\
	$(AM_CPPFLAGS)

minbar_CFLAGS = \
	   $(MINBAR_CFLAGS)	\
	   $(WARN_CFLAGS)		\
	   $(AM_CFLAGS)

minbar_LDFLAGS = \
	$(AM_LDFLAGS)

minbar_LDADD = @ITL_LIBS@ \
	$(MINBAR_LIBS)	\
 	$(INTLLIBS)

if USE_NOTIFY
minbar_LDADD += $(NOTIFY_LIBS)
minbar_CPPFLAGS += $(NOTIFY_CFLAGS)
endif


