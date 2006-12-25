SUBDIRS = src pixmaps data po doc

desktopdir = $(datadir)/applications
desktop_in_files = minbar.desktop

@INTLTOOL_DESKTOP_RULE@

distuninstallcheck_listfiles = find . -type f -print | grep -v scrollkeeper

EXTRA_DIST = \
	autogen.sh		\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	$(desktop_in_files)	

CLEANFILES = \
	$(desktop_DATA)

DISTCLEANFILES = \
	intltool-extract	\
	intltool-merge		\
	intltool-update
