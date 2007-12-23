

AC_INIT([minbar], [0.2.1],[mailto:djihed@gmail.com],[minbar])

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 no-dist-gzip -Wall])

AC_CONFIG_SRCDIR([src])

AC_PROG_CC
AC_HEADER_STDC

# *********
# Some vars
# *********
MAKKAH_LAT=21.43
MAKKAH_LON=39.77
AC_SUBST(MAKKAH_LON)
AC_SUBST(MAKKAH_LAT)

# *****
# GNOME
# *****

GNOME_COMMON_INIT
GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
GNOME_MAINTAINER_MODE_DEFINES

# ***************************
# Check for required packages
# ***************************

pkg_modules="gtk+-2.0 >= 2.10 glib-2.0 libglade-2.0 gstreamer-0.10 libgnomecanvas-2.0 libxml-2.0"
PKG_CHECK_MODULES(MINBAR, [$pkg_modules])

AC_CHECK_HEADER(itl/prayer.h,ITL_LIBS=-litl,AC_MSG_ERROR("itl/prayer.h not found."))
AC_SUBST(ITL_LIBS)

# libnotify check
LIBNOTIFY_REQUIRED=0.3.0
AC_ARG_ENABLE(libnotify,
            [AS_HELP_STRING([--disable-libnotify],
			   [Disable libnotify support])],,
	      enable_libnotify=auto)
if test "x$enable_libnotify" != "xno"; then
	PKG_CHECK_MODULES(NOTIFY,                            \
			  libnotify >= $LIBNOTIFY_REQUIRED,
			  have_libnotify=yes,
			  have_libnotify=no)
	if test "x$have_libnotify" = "xno" -a "x$enable_libnotify" = "xyes"; then
	  AC_MSG_ERROR([libnotify support explicitly requested, but libnotify couldn't be found])
	fi
	if test "x$have_libnotify" = "xyes"; then
	     enable_libnotify=yes
	fi
fi
AM_CONDITIONAL(USE_NOTIFY, test x"$enable_libnotify" = xyes)
if test x$enable_libnotify = xyes ; then
    # Find out the version of LIBNOTIFY we're using
    libnotify_version=`pkg-config --modversion libnotify`
    LIBNOTIFY_VERSION_MAJOR=`echo $libnotify_version | awk -F. '{print $1}'`
    LIBNOTIFY_VERSION_MINOR=`echo $libnotify_version | awk -F. '{print $2}'`
    LIBNOTIFY_VERSION_MICRO=`echo $libnotify_version | awk -F. '{print $3}'`
    if test "z$LIBNOTIFY_VERSION_MAJOR" = "z"; then
        LIBNOTIFY_VERSION_MAJOR="0"
    fi
    if test "z$LIBNOTIFY_VERSION_MINOR" = "z"; then
        LIBNOTIFY_VERSION_MINOR="0"
    fi
    if test "z$LIBNOTIFY_VERSION_MICRO" = "z"; then
        LIBNOTIFY_VERSION_MICRO="0"
    fi
    echo "Your libnotify version is $LIBNOTIFY_VERSION_MAJOR,$LIBNOTIFY_VERSION_MINOR,$LIBNOTIFY_VERSION_MICRO."
    NOTIFY_CFLAGS="$NOTIFY_CFLAGS -DLIBNOTIFY_VERSION_MAJOR=$LIBNOTIFY_VERSION_MAJOR"
    NOTIFY_CFLAGS="$NOTIFY_CFLAGS -DLIBNOTIFY_VERSION_MINOR=$LIBNOTIFY_VERSION_MINOR"
    NOTIFY_CFLAGS="$NOTIFY_CFLAGS -DLIBNOTIFY_VERSION_MICRO=$LIBNOTIFY_VERSION_MICRO"

    AC_DEFINE(HAVE_NOTIFY, 1, [Define if libnotify support is enabled])
    AC_SUBST(NOTIFY_CFLAGS)
    AC_SUBST(NOTIFY_LIBS)
fi

# *************
# Documentation
# *************

AC_PATH_PROG(SCROLLKEEPER_CONFIG, scrollkeeper-config,no)
if test x$SCROLLKEEPER_CONFIG = xno; then
  AC_MSG_ERROR(Couldn't find scrollkeeper-config, please install the scrollkeeper package)
fi

# ****************************************
# Preferences system (gconf or key-files)
# ****************************************
AC_ARG_ENABLE(gconf, [AS_HELP_STRING([--enable-gconf],
		[whether to use gconf or a keyfile for storing configuration 
		(by default use gconf if available)])],
		[gconf=$enableval],
		[gconf=auto])

PKG_CHECK_MODULES(GCONF, [gconf-2.0], gconf_available=yes, gconf_available=no)
AM_GCONF_SOURCE_2
if test x"$gconf" = "xyes"; then
	if test x"$gconf_available" = "xyes"; then
		AC_DEFINE([USE_GCONF], [1], [Define to 1 if you want to use gconf.])
	else
		AC_MSG_ERROR("You asked for the use of Gconf but it's not available")
	fi
elif test x"$gconf" = "xauto" && test x"$gconf_available" = "xyes"; then
	AC_DEFINE([USE_GCONF], [1], [Define to 1 if you want to use gconf.])
	gconf=yes
fi
AM_CONDITIONAL(USE_GCONF, test x"$gconf" = "xyes")

# ********************
# Internationalisation
# ********************

IT_PROG_INTLTOOL([0.34.90])

GETTEXT_PACKAGE=minbar
AC_SUBST([GETTEXT_PACKAGE])
AM_GLIB_GNU_GETTEXT
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[gettext domain])

# *************************************
# relative filenames (for windows)
# *************************************
AC_ARG_ENABLE(relative-filenames, [AS_HELP_STRING([--enable-relative-filenames],
		[whether to use gconf or a keyfile for storing configuration 
		(by default use gconf if available)])],
		[relative=$enableval],
		[relative=no])
if test x"$relative" = "xyes"; then
	adl_COMPUTE_RELATIVE_PATHS([bindir:datadir:data])
else
	data=$datadir
fi
AC_SUBST([data])

# *************************************
# *************************************

ATHANDIR="${data}/${PACKAGE}"
AC_SUBST(ATHANDIR)


#	data/minbar.schemas.in
AC_CONFIG_FILES([
	Makefile
	data/Makefile
	pixmaps/Makefile
	src/Makefile 
	po/Makefile.in
	doc/Makefile
	pixmaps/icons/Makefile
	pixmaps/icons/16x16/Makefile
	pixmaps/icons/24x24/Makefile
	pixmaps/icons/32x32/Makefile
	pixmaps/icons/48x48/Makefile
	pixmaps/icons/64x64/Makefile
	pixmaps/icons/128x128/Makefile
])

AC_OUTPUT

echo ""
echo ----------------------------------------------------------------------
echo "Configure Summary 

	Source code location .......:  ${srcdir}
	Compiler ...................:  ${CC} 
	Extra Compiler Warnings ....:  ${WARN_CFLAGS}
	Extra debugging support ....:  ${enable_debug}
"

if test x"$enable_libnotify" = xyes; then
	echo "	Libnotify notifications ....: Enabled (version $LIBNOTIFY_VERSION_MAJOR.\ 
$LIBNOTIFY_VERSION_MINOR.$LIBNOTIFY_VERSION_MICRO)."
else
	echo "	Libnotify notifications ....: Disabled."
fi
if test x"$gconf" = xyes; then
	echo "	Configuration ..............: gconf."
else
	echo "	Configuration ..............: gkeyfile."
fi
if test x"$relative" = xyes; then
	echo "	Using relative pathnames : minbar should be called from its directory."
fi
echo ----------------------------------------------------------------------
