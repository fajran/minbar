bin_PROGRAMS = minbar

minbar_SOURCES = main.c main.h defines.h prefs.h locations-xml.c locations-xml.h 

minbar_CPPFLAGS = 				\
	-DG_LOG_DOMAIN=\"Minbar\"		\
	-DGNOMELOCALEDIR=\""$(data)/locale"\"	\
	-DMINBAR_PIXMAPSDIR=\"$(data)/pixmaps\" \
	-DMINBAR_DATADIR=\"$(data)/minbar\"		\
	$(DISABLE_DEPRECATED)			\
	$(AM_CPPFLAGS)

minbar_CFLAGS = \
	   $(MINBAR_CFLAGS)	\
	   $(WARN_CFLAGS)	\
	   $(AM_CFLAGS) 	\
	   $(RSVG_CFLAGS)

minbar_LDFLAGS = \
	$(AM_LDFLAGS) -export-dynamic

minbar_LDADD = @ITL_LIBS@ 	\
	$(MINBAR_LIBS)		\
	$(INTLLIBS)		\
	$(RSVG_LIBS)

if USE_NOTIFY
minbar_LDADD += $(NOTIFY_LIBS)
minbar_CPPFLAGS += $(NOTIFY_CFLAGS)
endif

if USE_GCONF
minbar_LDADD += $(GCONF_LIBS)
minbar_CPPFLAGS += $(GCONF_CFLAGS)
endif

if USE_GSTREAMER
minbar_LDADD += $(GSTREAMER_LIBS)
minbar_CPPFLAGS += $(GSTREAMER_CFLAGS)
else
minbar_LDADD += $(XINE_LIBS)
minbar_CPPFLAGS += $(XINE_CFLAGS)
endif