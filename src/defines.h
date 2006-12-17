#define GPRAYER_DEFAULT_ATHAN "athan.ogg"
#define GLADE_MAIN_INTERFACE  "gprayer.glade"

/* Pictures, Icons, Graphics, etc */

#define GPRAYER_KAABA_ICON	"kaabatrayicon.png"
#define GPRAYER_BIG_LOGO	"gprayerlogo.png"
#define GPRAYER_QURAN		"quran.png"

/* Markup for time label */ 
#define MARKUP_NORMAL_START	"<span color='blue'><b>"
#define	MARKUP_NORMAL_END 	"</b></span>"
#define	MARKUP_SPECIAL_START 	"<span color='red'><b>"
#define	MARKUP_SPECIAL_END 	"</b></span>"
#define	MARKUP_FAINT_START 	"<span color='skyblue'><b>"
#define	MARKUP_FAINT_END 	"</b></span>"
#define	DATE_MARKUP_START 	"<span color='darkgreen'><b>"
#define	DATE_MARKUP_END 	"</b></span>"
#define	REMAIN_MARKUP_START 	"<span color='darkgreen'><b>"
#define	REMAIN_MARKUP_END 	"</b></span>"

/* gettext shortcut */
#ifndef __SLICE_INTL_H__
#define __SLICE_INTL_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS is disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain) 
#define bind_textdomain_codeset(Domain,Codeset) (Codeset) 
#endif /* ENABLE_NLS */

#endif /* __SLICE_INTL_H__ */
