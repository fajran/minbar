bin_PROGRAMS = minbar

# TODO: I think the headers should go in noinst_HEADERS
minbar_SOURCES = main.c \
		locations-xml.c locations-xml.h 

noinst_HEADERS = defines.h	\
		 main.h		\
		 prefs.h


#	-I$(top_srcdir)				
minbar_CPPFLAGS = 				\
	-DG_LOG_DOMAIN=\"Minbar\"		\
 	-DGNOMELOCALEDIR=\""$(data)/locale"\"	\
	-DMINBAR_PIXMAPSDIR=\"$(data)/pixmaps\" \
	-DMINBAR_DATADIR=\"$(data)\"		\
	$(DISABLE_DEPRECATED)			\
	$(AM_CPPFLAGS)

#	-DDATADIR=\""$(pkgdatadir)"\"			\
#	-DMINBAR_GLADEDIR=\"$(datadir)/$(PACKAGE)\" \
#	-DMINBAR_ATHAN_DIR=\"$(datadir)/$(PACKAGE)\" \
#

minbar_CFLAGS = \
	   $(MINBAR_CFLAGS)	\
	   $(WARN_CFLAGS)	\
	   $(AM_CFLAGS) 

minbar_LDFLAGS = \
	$(AM_LDFLAGS) -export-dynamic

minbar_LDADD = @ITL_LIBS@ 	\
	$(MINBAR_LIBS)		\
 	$(INTLLIBS)

if USE_NOTIFY
minbar_LDADD += $(NOTIFY_LIBS)
minbar_CPPFLAGS += $(NOTIFY_CFLAGS)
endif

if USE_GCONF
minbar_LDADD += $(GCONF_LIBS)
minbar_CPPFLAGS += $(GCONF_CFLAGS)
endif

