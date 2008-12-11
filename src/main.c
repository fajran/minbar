/* main.c - main program
 *
 * Copyright (C) 2006-2007 
 * 		Djihed Afifi <djihed@gmail.com>,
 * 		Abderrahim Kitouni <a.kitouni@gmail.com> 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <itl/prayer.h>
#include <itl/hijri.h>
#include <glib/gi18n.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "defines.h"
#include "prefs.h"
#include "locations-xml.h" 


#define USE_TRAY_ICON   1
#define USE_NOTIFY	(USE_TRAY_ICON & HAVE_NOTIFY)

#if USE_GSTREAMER
#include <gst/gst.h>
#else
#include <xine.h>
#endif

#if USE_NOTIFY
#include <libnotify/notify.h>
#endif

#if USE_RSVG
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#endif

#include "main.h"

/* Preferences */ 
static const gchar * 	program_name ;
static gfloat 		lat;
static gfloat 		height;
static gfloat 		lon;
static gchar 		* city_name;
static gboolean 	enable_athan;
static double 		correction = 0.0;
static gboolean		notif;
static int		notiftime;
static int 		method;
static int 		next_prayer_id = -1;
static gboolean 	start_hidden;
static gboolean		close_closes;
static gchar *		athan_file;
static gchar * 		athan_subh_file;
static gboolean 	* start_hidden_arg = FALSE;

/* for prayer.h functions */
static Date 		* prayerDate;
static Location		* loc;
static Method		* calcMethod;
static Prayer 		ptList[6];

/* For libraries */
#if USE_GCONF
static GConfClient	* client;
#else
static GKeyFile		* conffile;
#endif
static GladeXML		* xml;
static GError		* err 	= NULL;
#if USE_GSTREAMER
/* For gstreamer */
/*static GstElement	*pipeline, *source, *parser, *decoder, *conv, *sink;*/
static GstElement	*pipeline;
static GMainLoop	*loop;
static GstBus		*bus;
#else
static xine_t		*xine;
static xine_audio_port_t*audio_port;
static xine_stream_t	*stream = NULL;
#endif

static GtkFileFilter 	*filter_all;
static GtkFileFilter 	*filter_supported;

/* tray icon */
#if USE_TRAY_ICON
static GtkStatusIcon   	* status_icon;	
#endif
static GDate		* currentDate;

sDate 			* hijri_date;
static gchar 		* next_prayer_string;
static int 		calling_athan_for;
/* init moved for i18n */
gchar * hijri_month[13];
gchar * time_names[6];

#if USE_NOTIFY
NotifyNotification 	* notification;
#endif

/* qibla */
GtkWidget * qibla_drawing_area;
#if USE_RSVG
RsvgHandle * compass;
#endif

#if USE_TRAY_ICON
inline void set_status_tooltip()
{
	gchar * tooltiptext;
	tooltiptext = g_malloc(2000);
	g_snprintf(tooltiptext, 2000, "    %s \t\n\n"
					" %s:  %02d:%02d \n"
				 	" %s:  %02d:%02d \n"
					" %s:  %02d:%02d \n"
					" %s:  %02d:%02d \n"
					" %s:  %02d:%02d \n"
					" %s:  %02d:%02d"
					"\n\n"
					" %s   "
					,
					program_name,
			time_names[0], ptList[0].hour, ptList[0].minute,
			time_names[1], ptList[1].hour, ptList[1].minute,
			time_names[2], ptList[2].hour, ptList[2].minute,
			time_names[3], ptList[3].hour, ptList[3].minute,
			time_names[4], ptList[4].hour, ptList[4].minute,
			time_names[5], ptList[5].hour, ptList[5].minute, 
			next_prayer_string
		  );
	gtk_status_icon_set_tooltip(status_icon, tooltiptext);
	g_free(tooltiptext);

}
#endif

void update_remaining()
{
	/* converts times to minutes */
	int next_minutes = ptList[next_prayer_id].minute + ptList[next_prayer_id].hour*60;
	time_t 	result;
	struct 	tm * curtime;

	result 	= time(NULL);
	curtime = localtime(&result);
	int cur_minutes = curtime->tm_min + curtime->tm_hour * 60; 
	if(ptList[next_prayer_id].hour < curtime->tm_hour)
	{
		/* salat is on next day (subh, and even Isha sometimes) after midnight */
		next_minutes += 60*24;
	}

	int difference = next_minutes - cur_minutes;
	int hours = difference / 60;
	int minutes = difference % 60;

	gchar * trbuf; /* Formatted for display within applet */
			/* leaving next_prayer_string unformatted for tooltip */
	trbuf = g_malloc(600);

	if (difference == 0)
	{
		g_snprintf(next_prayer_string, 400,
			_("Time for prayer: %s"), 
			time_names[next_prayer_id]);
	}
	else if (difference < 60 )
	{
		g_snprintf(next_prayer_string, 400,
				_("%d %s until %s prayer."),
				minutes,
				ngettext("minute", "minutes", minutes),
				time_names[next_prayer_id]);
	}
	else if (difference % 60 == 0)
	{
		g_snprintf(next_prayer_string, 400,
				_("%d %s until %s prayer."),
				hours,
				ngettext("hour", "hours", hours),
				time_names[next_prayer_id]);
	}
	else
	{
		g_snprintf(next_prayer_string, 400,
				_("%d %s and %d %s until %s prayer."),
				hours,
				ngettext("hour", "hours", hours),
				minutes,
				ngettext("minute", "minutes", minutes),
				time_names[next_prayer_id]);
	}
	g_snprintf(trbuf, 600,
			("%s%s%s"),
			REMAIN_MARKUP_START,
			next_prayer_string,
			REMAIN_MARKUP_END);

	gtk_label_set_markup((GtkLabel *) glade_xml_get_widget(xml, 
				"timeleftlabel"), trbuf);
	g_free(trbuf);
}

void update_date_label()
{
	gchar  *miladi, * dateString, * weekday, * time_s;
	dateString 	= g_malloc(500);
	miladi 		= g_malloc(200);
	weekday		= g_malloc(50);
	time_s		= g_malloc(50);

	g_date_strftime(weekday, 50, "%A", currentDate);

	time_t 	result;
	struct 	tm * curtime;
	result 	= time(NULL);
	curtime = localtime(&result);


	/* TRANSLATOR: this is a format string for strftime
	 *             see `man 3 strftime` for more details
	 *             copy it if you're unsure
	 *             This will print an example: 12 January 2007
	 */
	if (!g_date_strftime (miladi, 200, _("%d %B %G"), currentDate))
	{
		g_date_strftime (miladi, 200, "%d %B %G", currentDate);
	}

	/* TRANSLATOR: this is a format string for strftime
	 *             see `man 3 strftime` for more details
	 *             copy it if you're unsure
	 *             This will print an example: 19:17.
	 *             if you want to use 12 hour format, use: %I:%M %p
	 *             which will print something similar to: 7:17 pm  
	 */
	if (!strftime (time_s, 50, _("%H:%M"), curtime))
	{
		strftime (time_s, 50, "%H:%M", curtime);
	}
	
	hijri_date 	= g_malloc(sizeof(sDate));
	h_date(hijri_date, prayerDate->day, prayerDate->month, prayerDate->year);
	g_snprintf(dateString, 500, "%s %s%s %d %s %d \n %s \n%s%s", DATE_MARKUP_START,
	weekday, /* The comma may differ from language to language*/ _(","),
	hijri_date->day, hijri_month[hijri_date->month], 
	hijri_date->year, 
	miladi, 
	time_s,
	DATE_MARKUP_END);

	gtk_label_set_markup((GtkLabel *)glade_xml_get_widget(xml, 
				"currentdatelabel"), dateString);
	g_free(dateString);
	g_free(hijri_date);
	g_free(miladi);
	g_free(weekday);
}

void calculate_prayer_table()
{
	/* Update the values */
	loc->degreeLat 		= lat;
	loc->degreeLong 	= lon;
	loc->gmtDiff		= correction;	
	getPrayerTimes (loc, calcMethod, prayerDate, ptList);	
	next_prayer();
	update_remaining();
}

void play_events()
{
	time_t 	result;
	struct 	tm * curtime;
	result 	= time(NULL);
	curtime = localtime(&result);

	int cur_minutes = curtime->tm_hour * 60 + curtime->tm_min;

	int i;
	for (i = 0; i < 6; i++)
	{
		if ( i == 1 ) { continue ;} /* skip shorouk */
		/* covert to minutes */
		int pt_minutes = ptList[i].hour*60 + ptList[i].minute;
#if USE_NOTIFY		
		if ((cur_minutes + notiftime == pt_minutes ) && notif)
		{
			gchar * message;
			message = g_malloc(400);
			g_snprintf(message, 400, _("%d minutes until %s prayer."), 
					notiftime, time_names[i]); 
			show_notification(message);
			g_free(message);
		}
#endif
		if (cur_minutes == pt_minutes)
		{
			calling_athan_for = i;
			if(enable_athan){play_athan_callback();}
#if USE_NOTIFY
			if(notif)
			{
				gchar * message;
				message = g_malloc(400);
				g_snprintf(message, 400, _("It is time for %s prayer."), time_names[i]); 

				show_notification(message);
				g_free(message);
			}
#endif		
		}
	}
}

void next_prayer()
{	
	/* current time */
	time_t result;
	struct tm * curtime;
	result 		= time(NULL);
	curtime 	= localtime(&result);

	int i;
	for (i = 0; i < 6; i++)
	{
		if ( i == 1 ) { continue ;} /* skip shorouk */
		next_prayer_id = i;
		if(ptList[i].hour > curtime->tm_hour || 
		  	(ptList[i].hour == curtime->tm_hour && 
		   	ptList[i].minute >= curtime->tm_min))
		{
			return;
		}
	}

	next_prayer_id = 0;	
}

void update_date()
{
	GTimeVal * curtime 	= g_malloc(sizeof(GTimeVal));

	currentDate 		= g_date_new();
	g_get_current_time(curtime);
	g_date_set_time_val(currentDate, curtime);
	g_free(curtime);

	/* Setting current day */
	prayerDate 		= g_malloc(sizeof(Date));
	prayerDate->day 	= g_date_get_day(currentDate);
	prayerDate->month 	= g_date_get_month(currentDate);
	prayerDate->year 	= g_date_get_year(currentDate);
	update_date_label();
	g_free(currentDate);
}

void update_calendar()
{
	gtk_calendar_select_month((GtkCalendar *) glade_xml_get_widget(xml, "prayer_calendar"),
			prayerDate->month - 1, prayerDate->year);
	gtk_calendar_select_day((GtkCalendar *) glade_xml_get_widget(xml, "prayer_calendar"),
			prayerDate->day);
}

void prayer_calendar_callback()
{
	guint * year = g_malloc(sizeof(guint));
	guint * month = g_malloc(sizeof(guint));
	guint * day = g_malloc(sizeof(guint));

	gtk_calendar_get_date((GtkCalendar *) glade_xml_get_widget(xml, "prayer_calendar"),
			year, month, day);

	Prayer calendarPtList[6];
	Date * cDate;	
	cDate 		= g_malloc(sizeof(Date));
	cDate->day 	= (int) *day;
	cDate->month 	= (int) (*month) +1;
	cDate->year 	= (int) *year;

	getPrayerTimes (loc, calcMethod, cDate, calendarPtList);
	g_free(cDate);
	update_prayer_labels(calendarPtList, "salatlabelc", FALSE);
	g_free(year);
	g_free(month);
	g_free(day);
}

/* This is cool: this will change the label next to the notification 
 * spin button the preferences window according to the value of 
 * the spin button
 * */
void minute_label_callback(GtkWidget *widget, gpointer user_data)
{
	gtk_label_set_text((GtkLabel *) 
		glade_xml_get_widget(xml, "minutes_label"),
		ngettext("minute", "minutes", 
			gtk_spin_button_get_value((GtkSpinButton *)
				widget))
		);
}

void update_prayer_labels(Prayer * ptList, gchar * prefix, gboolean coloured)
{
	/* getting labels and putting time strings */
	gchar * timestring;
	gchar * timelabel;
	timestring 	= g_malloc(50);
	timelabel	= g_malloc(20);

	int i;
	for (i=0; i < 6; i++)
	{
		g_snprintf(timelabel, 20, "%s%d", prefix, i);
		if( i == 1 && coloured)
		{
			g_snprintf(timestring, 50, "%s%02d:%02d%s", 
				MARKUP_FAINT_START, ptList[i].hour, 
				ptList[i].minute, MARKUP_FAINT_END);
		}
		else if ( i == next_prayer_id && coloured)
		{
			g_snprintf(timestring, 50, "%s%02d:%02d%s", 
				MARKUP_SPECIAL_START, ptList[i].hour, 
				ptList[i].minute, MARKUP_SPECIAL_END);
		}
		else
		{
			g_snprintf(timestring, 50, "%s%02d:%02d%s", 
				MARKUP_NORMAL_START, ptList[i].hour, 
				ptList[i].minute, MARKUP_NORMAL_END);
		}
	
		gtk_label_set_markup((GtkLabel *) glade_xml_get_widget(xml, timelabel),
				timestring);
	}
	
	g_free(timestring);
	g_free(timelabel);
}

/* needed post loading preferences.*/
void init_vars()
{
	/* Allocate memory for variables */
	loc 			= g_malloc(sizeof(Location));
	next_prayer_string = g_malloc(400);
		
	
	update_date();

	/* Location variables */
	loc->degreeLat 		= lat;
	loc->degreeLong 	= lon;
	loc->gmtDiff 		= correction;
	loc->dst		= 0;
	loc->seaLevel 		= 0;
	loc->pressure 		= 1010;
	loc->temperature	= 10;
}

void on_enabledathanmenucheck_toggled_callback(GtkWidget *widget,
				gpointer user_data)
{
	enable_athan = gtk_check_menu_item_get_active((GtkCheckMenuItem * ) widget);

	gtk_toggle_button_set_active((GtkToggleButton * )
			glade_xml_get_widget( xml, "enabledathancheck"),
			enable_athan);
	gtk_check_menu_item_set_active((GtkCheckMenuItem * )
			glade_xml_get_widget( xml, "playathan"),
			enable_athan);

#if USE_GCONF
	gconf_client_set_bool(client, PREF_PREF_PLAY, 
				enable_athan, &err);
#else
	g_key_file_set_boolean(conffile, "prefs", "play", enable_athan);
#endif
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
}

void on_enabledathancheck_toggled_callback(GtkWidget *widget,
				gpointer user_data)
{
	enable_athan = gtk_toggle_button_get_active((GtkToggleButton * ) widget);
	
	gtk_toggle_button_set_active((GtkToggleButton * )
			glade_xml_get_widget( xml, "enabledathancheck"),
			enable_athan);
	gtk_check_menu_item_set_active((GtkCheckMenuItem * )
			glade_xml_get_widget( xml, "playathan"),
			enable_athan);

#if USE_GCONF
	gconf_client_set_bool(client, PREF_PREF_PLAY, 
				enable_athan, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
#else
	g_key_file_set_boolean(conffile, "prefs", "play", enable_athan);
#endif
}


void on_notifmenucheck_toggled_callback(GtkWidget *widget, 
		gpointer user_data)
{
	notif = gtk_check_menu_item_get_active((GtkCheckMenuItem * ) widget);

	gtk_toggle_button_set_active((GtkToggleButton * )
			glade_xml_get_widget( xml, "yesnotif"),
			
			notif);
	gtk_check_menu_item_set_active((GtkCheckMenuItem * )
			glade_xml_get_widget( xml, "notifmenucheck"),
			notif);

#if USE_GCONF
	gconf_client_set_bool(client, PREF_PREF_NOTIF, 
				notif, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

#else
	g_key_file_set_boolean(conffile, "prefs", "notif", notif);
#endif
}

void on_editcityokbutton_clicked_callback(GtkWidget *widget,
	       				gpointer user_data) 
{

	GtkWidget*  entrywidget;	
	/* Setting what was found to editcity dialog*/
	entrywidget 	= glade_xml_get_widget( xml, "longitude");	
	lon 		=  gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget 	= glade_xml_get_widget( xml, "latitude");
	lat 		=  gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget 	= glade_xml_get_widget( xml, "cityname");
	g_stpcpy(city_name, gtk_entry_get_text((GtkEntry *)entrywidget)); 

	entrywidget 	= glade_xml_get_widget( xml, "correction");
	correction 	=  (double)gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget 	= glade_xml_get_widget( xml, "yesnotif");
	notif 		=  gtk_toggle_button_get_active((GtkToggleButton *)entrywidget);

	entrywidget 	= glade_xml_get_widget( xml, "notiftime");
	notiftime 	=  (int)gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget 	= glade_xml_get_widget( xml, "startHidden");
	start_hidden 	=  gtk_toggle_button_get_active((GtkToggleButton *)entrywidget);

	entrywidget 	= glade_xml_get_widget( xml, "methodcombo");
	method 		=  (int)gtk_combo_box_get_active((GtkComboBox *)entrywidget)  + 1;

	if(method < 0 || method > 6 ) { method = 5; }
	getMethod(method, calcMethod);

#if USE_GCONF
        /* set gconf settings */

	gconf_client_set_float(client, PREF_CITY_LAT, lat, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gconf_client_set_float(client, PREF_CITY_LON, lon, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gconf_client_set_string(client, PREF_CITY_NAME, 
						city_name, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gconf_client_set_float(client, PREF_CITY_CORRECTION, 
						correction, &err);

	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gconf_client_set_bool(client, PREF_PREF_NOTIF, notif, &err);

	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gconf_client_set_bool(client, PREF_PREF_HIDDEN, start_hidden, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gconf_client_set_int(client, PREF_PREF_NOTIF_TIME, notiftime, &err);

	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gconf_client_set_int(client, PREF_PREF_METHOD, method, &err);

	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gconf_client_set_string(client, PREF_ATHAN_SUBH, 
			gtk_file_chooser_get_filename ((GtkFileChooser *) (glade_xml_get_widget(xml, "athan_subh_chooser")))
			, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gconf_client_set_string(client, PREF_ATHAN_NORMAL, 
			gtk_file_chooser_get_filename ((GtkFileChooser *) (glade_xml_get_widget(xml, "athan_chooser")))
			, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
#else
	g_key_file_set_double(conffile, "city", "latitude", lat);	
	g_key_file_set_double(conffile, "city", "longitude", lon);
	g_key_file_set_string(conffile, "city", "name", city_name);
	g_key_file_set_double(conffile, "city", "height", height);
	g_key_file_set_double(conffile, "prefs", "correction", correction);

	g_key_file_set_boolean(conffile, "prefs", "play", enable_athan);
	g_key_file_set_boolean(conffile, "prefs", "starthidden", start_hidden);
	g_key_file_set_boolean(conffile, "prefs", "notif", notif);
	g_key_file_set_integer(conffile, "prefs", "notiftime", notiftime);

	g_key_file_set_integer(conffile, "prefs", "method", method);

	g_key_file_set_string(conffile, "athan", "normal", 
			gtk_file_chooser_get_filename ((GtkFileChooser *) (glade_xml_get_widget(xml, "athan_chooser")))
			);
	g_key_file_set_string(conffile, "athan", "subh", 
			gtk_file_chooser_get_filename ((GtkFileChooser *) (glade_xml_get_widget(xml, "athan_subh_chooser")))
			);

	// write this config to the file
	gsize len;
	gchar* data = g_key_file_to_data (conffile, &len, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gchar * file = g_build_filename(g_get_user_config_dir(), "minbar", "prefs.conf", NULL);
	g_file_set_contents(file, data, len, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	g_free(data);
	g_free(file);
#endif

	/* Now hide the cityedit dialog */
	gtk_widget_hide(glade_xml_get_widget( xml, "editcity"));

	/* And set the city string in the main window */
	gtk_label_set_text((GtkLabel *)
			(glade_xml_get_widget(xml, "locationname"))
			,(const gchar *)city_name);

	/* Now calculate new timetable */
	calculate_prayer_table();
	/* And set the new labels */
	update_prayer_labels(ptList, "salatlabel", TRUE);
	gtk_widget_queue_draw(qibla_drawing_area);
	prayer_calendar_callback();
}


void init_prefs ()
{
#if USE_GCONF
	lat 	= gconf_client_get_float(client, PREF_CITY_LAT, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	lon 	= gconf_client_get_float(client, PREF_CITY_LON, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	city_name = gconf_client_get_string(client, PREF_CITY_NAME, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	height 	= gconf_client_get_float(client, PREF_CITY_HEIGHT, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	enable_athan  = gconf_client_get_bool(client, PREF_PREF_PLAY, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	correction  = gconf_client_get_float(client, PREF_CITY_CORRECTION, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	method  = gconf_client_get_int(client, PREF_PREF_METHOD, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	if( method < 0 || method > 6)
	{
		g_printerr(_("Invalid calculation method in preferences, using 5: Muslim world League \n"));
	}
	start_hidden  = gconf_client_get_bool(client, PREF_PREF_HIDDEN, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	calcMethod 		= g_malloc(sizeof(Method));
	getMethod(method, calcMethod);


	notif  = gconf_client_get_bool(client, PREF_PREF_NOTIF, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	notiftime  = gconf_client_get_int(client, PREF_PREF_NOTIF_TIME, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	close_closes  = gconf_client_get_bool(client, PREF_PREF_CLOSES, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	athan_file  = gconf_client_get_string(client, PREF_ATHAN_NORMAL, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	athan_subh_file  = gconf_client_get_string(client, PREF_ATHAN_SUBH, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

#else

	lat = g_key_file_get_double(conffile, "city", "latitude", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	lon 	= g_key_file_get_double(conffile, "city", "longitude", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	city_name = g_key_file_get_string(conffile, "city", "name", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	height 	= g_key_file_get_double(conffile, "city", "height", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	enable_athan  = g_key_file_get_boolean(conffile, "prefs", "play", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	correction  = g_key_file_get_double(conffile, "prefs", "correction", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	method  = g_key_file_get_integer(conffile, "prefs", "method", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	if( method < 0 || method > 6)
	{
		g_printerr(_("Invalid calculation method in preferences, using 5: Muslim world League \n"));
	}
	start_hidden  = g_key_file_get_boolean(conffile, "prefs", "starthidden", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	calcMethod 		= g_malloc(sizeof(Method));
	getMethod(method, calcMethod);

	notif  = g_key_file_get_boolean(conffile, "prefs", "notif", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	notiftime  = g_key_file_get_integer(conffile, "prefs", "notiftime", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	close_closes  = g_key_file_get_boolean(conffile, "prefs", "closes", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	athan_file  = g_key_file_get_string(conffile, "athan", "normal", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	athan_subh_file  = g_key_file_get_string(conffile, "athan", "subh", &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
#endif

	GtkWidget*  entrywidget;	
	
	/* Setting what was found to editcity dialog*/
	entrywidget = glade_xml_get_widget( xml, "latitude");	
	gtk_spin_button_set_value((GtkSpinButton *)entrywidget, lat);

	entrywidget = glade_xml_get_widget( xml, "longitude");	
	gtk_spin_button_set_value((GtkSpinButton *)entrywidget, lon);

	entrywidget = glade_xml_get_widget( xml, "cityname");	
	gtk_entry_set_text((GtkEntry *)entrywidget, city_name);

	entrywidget = glade_xml_get_widget( xml, "correction");	
	gtk_spin_button_set_value((GtkSpinButton *)entrywidget, correction);

	entrywidget = glade_xml_get_widget( xml, "yesnotif");	
	gtk_toggle_button_set_active((GtkToggleButton *)entrywidget, notif);
	
	entrywidget = glade_xml_get_widget( xml, "notiftime");	
	gtk_spin_button_set_value((GtkSpinButton *)entrywidget, notiftime);

	entrywidget = glade_xml_get_widget( xml, "methodcombo");	
	gtk_combo_box_set_active((GtkComboBox *)entrywidget, method-1);

	/* Set the play athan check box */
	entrywidget = glade_xml_get_widget( xml, "enabledathancheck");
	gtk_toggle_button_set_active((GtkToggleButton *) entrywidget, enable_athan);
	gtk_check_menu_item_set_active((GtkCheckMenuItem * )
			glade_xml_get_widget( xml, "playathan"),
			enable_athan);

	/* notitication menu item */
	gtk_check_menu_item_set_active((GtkCheckMenuItem * )
			glade_xml_get_widget( xml, "notifmenucheck"),
			notif);

	/* Start minimised checkbox */
	gtk_toggle_button_set_active((GtkToggleButton * )
			glade_xml_get_widget( xml, "startHidden"),
			start_hidden);

	/* And set the city string in the main window */
	gtk_label_set_text((GtkLabel *)
			(glade_xml_get_widget(xml, "locationname")),
		       	(const gchar *)city_name);

	/* show on start up? */
	GtkWidget * mainwindow = glade_xml_get_widget(xml, "mainWindow");
	
#if USE_TRAY_ICON
	if(!start_hidden && !start_hidden_arg)
	{
		gtk_widget_show(mainwindow);
	}
	else
	{
		gtk_widget_hide(mainwindow);
	}
#else
	gtk_widget_show(mainwindow);
#endif
	/* set UI vars */
	/* Check existence of file */
	FILE * testfile;
	testfile = fopen( athan_subh_file, "r");
	if(testfile != NULL)
	{
	fclose(testfile);
	gtk_file_chooser_set_filename  ((GtkFileChooser *) 
			(glade_xml_get_widget(xml, "athan_subh_chooser")),
			(const gchar *) athan_subh_file);
	}
	else
	{
		calling_athan_for = 0;
		set_file_status(FALSE);
	}
	setup_file_filters();
	gtk_file_chooser_add_filter ((GtkFileChooser *) 
			(glade_xml_get_widget(xml, "athan_subh_chooser")),
	       		filter_supported);

	gtk_file_chooser_add_filter ((GtkFileChooser *) 
			(glade_xml_get_widget(xml, "athan_subh_chooser")),
	       		filter_all);

	testfile = fopen( athan_file, "r");
	if(testfile != NULL)
	{

	fclose(testfile);
	gtk_file_chooser_set_filename  ((GtkFileChooser *) 
			(glade_xml_get_widget(xml, "athan_chooser")),
			(const gchar *) athan_file);
	}
	else
	{
		calling_athan_for = 1;
		set_file_status(FALSE);
	}
	setup_file_filters();
	gtk_file_chooser_add_filter ((GtkFileChooser *) 
			(glade_xml_get_widget(xml, "athan_chooser")),
	       		filter_supported);

	gtk_file_chooser_add_filter ((GtkFileChooser *) 
			(glade_xml_get_widget(xml, "athan_chooser")),
	       		filter_all);


}

gboolean no_stream_errors;
void play_athan_callback()
{

	/* Stop previously played file */
	stop_athan_callback();
#if USE_GSTREAMER
	int returned = init_pipelines();
	if(returned < 0)
	{
		exit(-1);
	}
#else
	stream = xine_stream_new(xine, audio_port, NULL);
#endif
	gchar * athanuri; 
	/* set filename property on the file source. Also add a message
	 * handler. */

	no_stream_errors = TRUE;
	if(calling_athan_for == 0)
	{
		athanuri  = gtk_file_chooser_get_uri  
		((GtkFileChooser *) (glade_xml_get_widget(xml, "athan_subh_chooser")));
	}
	else
	{
		athanuri  = gtk_file_chooser_get_uri  
		((GtkFileChooser *) (glade_xml_get_widget(xml, "athan_chooser")));
	}
#if USE_GSTREAMER
/*	g_object_set (G_OBJECT (source), "", athanfilename, NULL); */
	g_object_set (G_OBJECT (pipeline), "uri", athanuri, NULL);

	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
	gst_bus_add_watch (bus, bus_call, loop);
	gst_object_unref (bus);

	/* put all elements in a bin */
/*	gst_bin_add_many (GST_BIN (pipeline),
		    source, parser, decoder, conv, sink, NULL);
*/
	/* link together - note that we cannot link the parser and
	 * decoder yet, becuse the parser uses dynamic pads. For that,
	 * we set a pad-added signal handler. */
/*	gst_element_link (source, parser);
	gst_element_link_many (decoder, conv, sink, NULL);
	g_signal_connect (parser, "pad-added", G_CALLBACK (new_pad), NULL);
*/

	/* Now set to playing and iterate. */
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
	g_main_loop_run (loop);

  	/* clean up nicely */
  	gst_element_set_state (pipeline, GST_STATE_NULL);
  	gst_object_unref (GST_OBJECT (pipeline));
#else
	xine_open(stream, athanfilename);
	xine_play(stream, 0, 0);
#endif

}

void play_subh_athan_callback ()
{
	calling_athan_for = 0;
	play_athan_callback();
}

void play_normal_athan_callback ()
{
	calling_athan_for = 2;
	play_athan_callback();
}

void stop_athan_callback()
{
	/* clean up nicely */
#if USE_GSTREAMER
	if(GST_IS_ELEMENT (pipeline))
	{		
		gst_element_set_state (pipeline, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (pipeline));
	}
#else
	if (stream)
	{
	  xine_dispose(stream);
	  stream = NULL;
	}
#endif
}

#if USE_GSTREAMER

gboolean bus_call (GstBus     *bus,
	  GstMessage *msg,
	  gpointer    data)
{
	switch (GST_MESSAGE_TYPE (msg)) {
		case GST_MESSAGE_EOS:
			/* End of Stream */
			g_main_loop_quit (loop);
			break;
		case GST_MESSAGE_ERROR: {
			gchar *debug;
			GError *err;

			gst_message_parse_error (msg, &err, &debug);
			g_free (debug);

			g_print (_("Error: %s\n"), err->message);
			g_error_free (err);
			
			g_main_loop_quit (loop);
			no_stream_errors= FALSE;
			break;
		}
		default:
			break;
		}
	
	set_file_status(no_stream_errors);
	return TRUE;
}
#endif
void set_file_status(gboolean status)
{
	gchar * label_name = g_malloc(100);
	gchar * label_status = g_malloc(100);
	
	if(calling_athan_for == 0)
		strcpy(label_name, "athansubhstatusimage");
	else
		strcpy(label_name, "athanstatusimage");

	if(status)
		strcpy(label_status, GTK_STOCK_APPLY);
	else
		strcpy(label_status, GTK_STOCK_DIALOG_WARNING);

	gtk_image_set_from_stock((GtkImage *) (glade_xml_get_widget(xml, label_name)),
			label_status, GTK_ICON_SIZE_BUTTON); 

	g_free(label_name);
	g_free(label_status);
}

#if USE_GSTREAMER
/*void new_pad (GstElement *element,
	 	GstPad     *pad,
	 	gpointer    data)
{
	GstPad *sinkpad;
	sinkpad = gst_element_get_pad (decoder, "sink");
	gst_pad_link (pad, sinkpad);

	gst_object_unref (sinkpad);
}*/



int init_pipelines()
{
	/* create elements */
/*	pipeline 	= gst_pipeline_new ("audio-player");
	source 		= gst_element_factory_make ("filesrc", "file-source");
	parser 		= gst_element_factory_make ("oggdemux", "ogg-parser");
	decoder 	= gst_element_factory_make ("vorbisdec", "vorbis-decoder");
	conv 		= gst_element_factory_make ("audioconvert", "converter");
	sink 		= gst_element_factory_make ("alsasink", "alsa-output");
	if (!pipeline || !source || !parser || !decoder || !conv || !sink) {
		g_print ("One element could not be created\n");*/
	pipeline	= gst_element_factory_make ("playbin", "play");

	if (!pipeline) {
		g_print ("pipeline could not be created\n");
		return -1;
	}
	return 1;
}
#endif

void setup_file_filters (void)
{
	filter_all = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter_all, _("All files"));
	gtk_file_filter_add_pattern (filter_all, "*");
	g_object_ref (filter_all);

	filter_supported = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter_supported,
		_("Supported files"));
#if USE_GSTREAMER
	gtk_file_filter_add_mime_type (filter_supported, "application/ogg");
	gtk_file_filter_add_mime_type (filter_supported, "audio/*");
#else
	char* xine_supported = xine_get_mime_types(xine);
	char* result = strtok(xine_supported, ":");
	while (result != NULL)
	{
	  gtk_file_filter_add_mime_type (filter_supported, result);
	  strtok(NULL, ";");
	  result = strtok(NULL, ":");
	}
#endif
	g_object_ref (filter_supported);
}

/* Interval to update prayer times if time/date changes */
gboolean update_interval(gpointer data)
{
	update_date(); 
	calculate_prayer_table(); 
	update_prayer_labels(ptList, "salatlabel", TRUE);
	
	play_events();
#if USE_TRAY_ICON
	set_status_tooltip();
#endif
	
	return TRUE;
}

/* System tray icon */
#if USE_TRAY_ICON
void load_system_tray()
{
	status_icon 	= gtk_status_icon_new_from_icon_name
		("minbar");
	
	g_signal_connect ((GtkStatusIcon * ) (status_icon), "popup_menu", 
			G_CALLBACK(tray_icon_right_clicked_callback) , NULL);
	g_signal_connect ((GtkStatusIcon * ) (status_icon), "activate", 
			G_CALLBACK(tray_icon_clicked_callback) , NULL);	
}
#endif


void check_quit_callback(GtkWidget *widget, gpointer data)
{
	if(close_closes)
		gtk_main_quit();
	else
		gtk_widget_hide(glade_xml_get_widget(xml, "mainWindow"));
}

void quit_callback ( GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

void tray_icon_right_clicked_callback (GtkWidget *widget, gpointer data)
{
	GtkMenu * popup_menu = (GtkMenu * )(glade_xml_get_widget(xml, "traypopup")); 
	
	gtk_menu_set_screen (GTK_MENU (popup_menu), NULL);
	
	gtk_menu_popup (GTK_MENU (popup_menu), NULL, NULL, NULL, NULL,
			2, gtk_get_current_event_time());
}

void show_window_clicked_callback (GtkWidget *widget, gpointer data)
{
	if(GTK_WIDGET_VISIBLE(glade_xml_get_widget(xml, "mainWindow")))
	{
		gtk_widget_hide(glade_xml_get_widget(xml, "mainWindow"));
	}
	else
	{
		gtk_window_present((GtkWindow *)glade_xml_get_widget(xml, "mainWindow"));
	}
}

void tray_icon_clicked_callback ( GtkWidget *widget, gpointer data)
{
	if(gtk_window_is_active((GtkWindow *)glade_xml_get_widget(xml, "mainWindow")))
	{
		gtk_widget_hide(glade_xml_get_widget(xml, "mainWindow"));
	}
	else
	{
		gtk_window_present((GtkWindow *)glade_xml_get_widget(xml, "mainWindow"));
	}
}

/* quit callback */
void close_callback( GtkWidget *widget,
	    gpointer data)
{
	gtk_widget_hide(glade_xml_get_widget(xml, "mainWindow"));

}

/**** Notification Balloons ****/
#if USE_NOTIFY
void show_notification(gchar * message)
{
	notify_notification_update(notification,
				program_name,
				message,
				GTK_STOCK_ABOUT);
	notify_notification_show(notification, NULL);
}

void create_notification()
{
	notification = notify_notification_new
                                            (program_name,
                                             NULL,
                                             NULL,
					     NULL);
	notify_notification_attach_to_status_icon (notification, status_icon );
	notify_notification_set_timeout (notification, 8000);
}
#endif

/**** Main ****/
int main(int argc, char *argv[]) 
{
	/* init gettext */

	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	program_name = _("Minbar Prayer Times");

	hijri_month[0]	= _("skip");
	hijri_month[1]	= _("Muharram");
	hijri_month[2]	= _("Safar");
	hijri_month[3]	= _("Rabi I");
	hijri_month[4]	= _("Rabi II");
	hijri_month[5]	= _("Jumada I");
	hijri_month[6]	= _("Jumada II");
	hijri_month[7]	= _("Rajab");
	hijri_month[8]	= _("Shaaban");
	hijri_month[9]	= _("Ramadan");
	hijri_month[10]	= _("Shawwal");
	hijri_month[11]	= _("Thul-Qiaadah");
	hijri_month[12]	= _("Thul-Hijja");

	time_names[0]	= _("Subh");
	time_names[1]	= _("Shorook");
	time_names[2]	= _("Dhuhr");
	time_names[3]	= _("Asr");
	time_names[4]	= _("Maghreb");
	time_names[5]	= _("Isha'a");

	/* init libraries */
	gtk_init(&argc, &argv);
	glade_init();

#if USE_GSTREAMER	
	/* initialize GStreamer */
	gst_init (&argc, &argv);
	loop = g_main_loop_new (NULL, FALSE);
#else
	xine = xine_new();
	xine_init(xine);
	audio_port = xine_open_audio_driver(xine , "auto", NULL);
#endif

	/* command line options */
	GOptionEntry options[] = 
	{
		{"hide", 'h', 0, G_OPTION_ARG_NONE, &start_hidden_arg, 	
			_("Hide main window on start up."), NULL },
		{ NULL }
	};
	
	GOptionContext *context = NULL;
	context = g_option_context_new (NULL);
  	g_option_context_add_main_entries (context, options, PACKAGE_NAME);
  	g_option_context_set_help_enabled (context, TRUE);

	g_option_context_parse (context, &argc, &argv, NULL);
  	g_option_context_free (context);
	
#if USE_NOTIFY
	notify_init(program_name);
#endif
#if USE_GCONF
	/* load gconf client */
	client = gconf_client_get_default();
#else
	conffile = g_key_file_new ();
	gchar * filename = g_build_filename(g_get_user_config_dir(),"minbar", "prefs.conf" ,NULL);
	g_key_file_load_from_file (conffile, filename, G_KEY_FILE_NONE, &err);
	g_free(filename);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
#endif

	gtk_about_dialog_set_url_hook (activate_url, NULL, NULL);

	/* load the interface */
	xml = glade_xml_new(g_build_filename(MINBAR_DATADIR,GLADE_MAIN_INTERFACE,NULL), NULL, NULL);
	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);
	
	/* Set up some widgets and options that are not stored in the glade xml */
	setup_widgets();
	
	/* System tray icon */
#if USE_TRAY_ICON
	load_system_tray();
#endif
		
	/* Initialize preferences and variables */	
	init_prefs();
	init_vars();

	/* calculate the time table, and update the labels, other inits */
	calculate_prayer_table();
	update_prayer_labels(ptList, "salatlabel", TRUE);
	update_calendar();
	prayer_calendar_callback();
	gtk_widget_queue_draw(qibla_drawing_area);
#if USE_TRAY_ICON
	/* set system tray tooltip text */
	set_status_tooltip();
#if USE_NOTIFY
	/* Used to balloon tray notifications */
	create_notification();
#endif
#endif
	/* start athan playing, time updating interval */
	g_timeout_add(60000, update_interval, NULL);

	/* start the event loop */
  	gtk_main();
	return 0;
}


gboolean draw_qibla (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	double height =  (double) widget->allocation.height;
	double width  =  (double) widget->allocation.width;

	double actual;

	/* use cairo */
	cairo_t *context = gdk_cairo_create(widget->window);

	double qibla = getNorthQibla(loc);
	/* it was deg, convert to rad */
	double qiblarad = - (qibla * G_PI) / 180;

	gchar * qiblabuf;

	cairo_save(context); /* for transforms */

	/* if the place is Makkah itself, don't draw */
	if((int)(lat * 10 ) == 214 && (int)(lon * 10 ) == 397) /* be less restrictive */
	{
	  qiblabuf = g_malloc(100);
	  g_snprintf(qiblabuf, 100,
		     "%s",
		     _("In Makkah!")
		     );
	}
	else
	{
#if USE_RSVG
		actual = height*580/680 < width ? height*580/680 : width;

		/* center the compass */
		cairo_translate(context, (width-actual)/2, (height-(actual*680/580))/2);
		cairo_scale(context, actual/580, actual/580);
		rsvg_handle_render_cairo_sub(compass, context, "#compass");

		/* the ibra */

		cairo_rotate(context, qiblarad);
		double dx = (388 * sin(qiblarad) + 290 * cos(qiblarad)) - 290;
		double dy = (388 * cos(qiblarad) - 290 * sin(qiblarad)) - 388;
		cairo_translate(context, dx, dy); /* recenter */

		rsvg_handle_render_cairo_sub(compass, context, "#ibra");
#else
		actual = height < width ? height : width;

		double nq = (actual - 10) / 2;

		/* transform: translate to center and make the y axis point to qibla */
		cairo_translate(context, width/2, height/2);
		cairo_rotate(context, qiblarad + G_PI);

		/* the circle background */
		cairo_set_source_rgba(context, 0, 0, 0, 0.08);
		cairo_arc(context, 0, 0, nq, 0, 2*G_PI);
		cairo_fill(context);

		/* the circle */
		cairo_set_source_rgb(context, 0, 1, 0);
		cairo_arc(context, 0, 0, nq, 0, 2*G_PI);
		cairo_stroke(context);

		/* the center dot */
		cairo_set_source_rgb(context, 0, 0, 1);
		cairo_arc(context, 0, 0, 2.5, 0, 2*G_PI);
		cairo_fill(context);

		/* the arrow */

		cairo_set_line_width(context, 2.0);
		cairo_move_to(context, 0, 0);
		cairo_line_to(context, 0, nq);
		cairo_stroke(context);

		cairo_move_to(context, 0, nq);
		cairo_rel_line_to(context, 3, -5);
		cairo_rel_line_to(context, -6, 0);
		cairo_close_path(context);
		cairo_fill(context);
#endif
	qiblabuf = g_malloc(300);
	g_snprintf(qiblabuf, 300,
		   "%s\n%d %s",
		   _("Qibla direction"),
		   (int) rint(fabs(qibla)),
		   (floor(qibla) == 0 || rint(fabs(qibla)) == 180) ? "" : qibla < 0 ? _("East") : _("West")
		   );
	}

	gtk_widget_set_tooltip_text(widget, qiblabuf);

	/* restore the transform */
	cairo_restore(context);

	g_free(qiblabuf);

	/* free the cairo context */
	cairo_destroy(context);
	return TRUE;
}

void window_state_event_callback (GtkWidget *widget, 
		GdkEventWindowState *event)
{
	if ((event->new_window_state & GDK_WINDOW_STATE_ICONIFIED) &&
		( event->changed_mask & GDK_WINDOW_STATE_ICONIFIED ))
	{	
			gtk_widget_hide(glade_xml_get_widget(xml, "mainWindow"));
	}
}

void
activate_url (GtkAboutDialog *about,
              const gchar    *link,
	      gpointer        data)
{
#ifdef G_OS_WIN32
  ShellExecuteA (0, "open", link, NULL, NULL, SW_SHOWNORMAL);
#else
  gchar *command = getenv("BROWSER");
  command = g_strdup_printf("%s '%s' &", command ? command : "gnome-open", link);
  system(command);
  g_free(command);
#endif
}

void setup_widgets()
{
	GtkWidget * mainwindow = glade_xml_get_widget(xml, "mainWindow");

	/* set the prayer names in the time table */
	/* done here so we don't duplicate translation */
	gchar * labeltext;
	labeltext = g_malloc(100);
	
	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[0]);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "subh"), labeltext);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "subhc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[1]);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "shourouk"), labeltext);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "shouroukc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[2]);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "duhr"), labeltext);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "duhrc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[3]);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "asr"), labeltext);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "asrc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[4]);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "maghreb"), labeltext);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "maghrebc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[5]);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "isha"), labeltext);
	gtk_label_set_markup((GtkLabel *)	glade_xml_get_widget(xml, "ishac"), labeltext);

	g_free(labeltext);

#if USE_TRAY_ICON
	/* hide on minimise*/	

	g_signal_connect (mainwindow, "window-state-event", 
			G_CALLBACK (window_state_event_callback), NULL);
#else
	GtkWidget * closebutton = glade_xml_get_widget(xml, "closebutton");
	gtk_widget_hide(closebutton);
#endif

#if USE_NOTIFY
	/* Hide install notice */
	GtkWidget * label = glade_xml_get_widget(xml, "installnotifnotice");
	gtk_widget_hide((GtkWidget *) label);
#else
	/* disable notification options */
	GtkWidget * check = glade_xml_get_widget(xml, "yesnotif");
	gtk_widget_set_sensitive ( (GtkWidget *) check, FALSE);
	GtkWidget * notif_t =  glade_xml_get_widget(xml, "notiftime");
	gtk_widget_set_sensitive ( (GtkWidget *) notif_t, FALSE);
	GtkWidget * notif_c =  glade_xml_get_widget(xml, "notifmenucheck");
	gtk_widget_set_sensitive ( (GtkWidget *) notif_c, FALSE);
#endif

	qibla_drawing_area = (GtkWidget *)glade_xml_get_widget(xml, "qibla_drawing_area");
#if USE_RSVG
	gchar * filename = g_build_filename(MINBAR_DATADIR ,"Compass.svg", NULL);
	compass = rsvg_handle_new_from_file(filename, NULL); /* err */
	g_free(filename);
#endif
}

gboolean locations_loaded = FALSE;

void load_locations_callback()
{
	if (!locations_loaded)
	{
		 setup_locations_applet();
		 locations_loaded = TRUE;
	}
	gtk_widget_show(glade_xml_get_widget(xml, "locationsDialog"));
}

GtkWidget * tree;
void setup_locations_applet()
{
	GtkTreeStore *model;
	/*GtkTreeSelection *selection;*/
	GtkWidget *scrolled_window;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell_renderer;
	/*WeatherLocation *current_location;*/

       	scrolled_window	= (GtkWidget*) glade_xml_get_widget(xml, "location_list_scroll");

	model 		= gtk_tree_store_new (GWEATHER_XML_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
	tree 		= gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);

	/* Add a colum for the locations */
	cell_renderer 	= gtk_cell_renderer_text_new ();
	column 		= gtk_tree_view_column_new_with_attributes ("not used", cell_renderer,
				       "text", GWEATHER_XML_COL_LOC, NULL);
	gtk_tree_view_append_column ((GtkTreeView *)tree, column);
	gtk_tree_view_set_expander_column (GTK_TREE_VIEW (tree), column);

	gtk_container_add (GTK_CONTAINER (scrolled_window), tree);
	gtk_widget_show (tree);
	gtk_widget_show (scrolled_window);

	/* current_location = weather_location_clone (gw_applet->gweather_pref.location);*/ 
	/* load locations from xml file */
	if (gweather_xml_load_locations ((GtkTreeView *)tree, NULL))
	{
		GtkWidget *d;
		d = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
			    _("Failed to load the Locations XML "
			      "database.  Please report this as "
			      "a bug."));
		gtk_dialog_run (GTK_DIALOG (d));
		gtk_widget_destroy (d);
	}
}

void locationok_callback()
{
	if(!tree)
		return;

	GtkTreeSelection * selection;
	selection = gtk_tree_view_get_selection ((GtkTreeView *)tree);
		
	GtkTreeModel *model;	
	GtkTreeIter iter;

	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;

	WeatherLocation *loc = NULL;

	gtk_tree_model_get (model, &iter, GWEATHER_XML_COL_POINTER, &loc, -1);

	if (!loc)
		return;
	gtk_widget_hide(glade_xml_get_widget(xml, "locationsDialog"));
	/*g_print("%s, %f,%f \n", loc->name, loc->longitude, loc->latitude);*/

	lat = (loc->latitude * 180) / M_PI;
	lon = (loc->longitude * 180) / M_PI;
	city_name = g_strdup(loc->name);  /* is this ok or should I use g_strdup ? */

	/* update the editcity dialog (copied from init_vars) */

		GtkWidget*  entrywidget;	

	/* Setting what was found to editcity dialog*/
	entrywidget = glade_xml_get_widget( xml, "latitude");	
	gtk_spin_button_set_value((GtkSpinButton *)entrywidget, lat);

	entrywidget = glade_xml_get_widget( xml, "longitude");	
	gtk_spin_button_set_value((GtkSpinButton *)entrywidget, lon);

	entrywidget = glade_xml_get_widget( xml, "cityname");	
	gtk_entry_set_text((GtkEntry *)entrywidget, city_name);
}



void
find_entry_changed (GtkEditable *entry/*, GWeatherPref *pref*/)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkWidget *nextbutton;
	const gchar *location;

	nextbutton = (GtkWidget *) glade_xml_get_widget(xml, "findnextbutton");

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(tree));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
	gtk_tree_model_get_iter_first (model, &iter);

	location = gtk_entry_get_text (GTK_ENTRY (entry));

	if (find_location (model, &iter, location, TRUE)) 
	{
		gtk_widget_set_sensitive (nextbutton , TRUE);

		path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_view_expand_to_path (GTK_TREE_VIEW(tree), path);
		gtk_tree_selection_select_iter (selection, &iter);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(tree), path, NULL, TRUE, 0.5, 0);

		gtk_tree_path_free (path);
	} 
	else 
	{
		gtk_widget_set_sensitive (nextbutton, FALSE);
	}
}


/* shamelessly copied from gweather code */
gboolean
find_location (GtkTreeModel *model, GtkTreeIter *iter, const gchar *location, gboolean go_parent)
{
	GtkTreeIter iter_child;
	GtkTreeIter iter_parent;
	gchar *aux_loc;
	gboolean valid;
	int len;

	len = strlen (location);

	if (len <= 0) {
		return FALSE;
	}
	
	do {
		
		gtk_tree_model_get (model, iter, GWEATHER_XML_COL_LOC, &aux_loc, -1);

		if (g_ascii_strncasecmp (aux_loc, location, len) == 0) {
			g_free (aux_loc);
			return TRUE;
		}

		if (gtk_tree_model_iter_has_child (model, iter)) {
			gtk_tree_model_iter_nth_child (model, &iter_child, iter, 0);

			if (find_location (model, &iter_child, location, FALSE)) {
				/* Manual copying of the iter */
				iter->stamp = iter_child.stamp;
				iter->user_data = iter_child.user_data;
				iter->user_data2 = iter_child.user_data2;
				iter->user_data3 = iter_child.user_data3;

				g_free (aux_loc);
				
				return TRUE;
			}
		}
		g_free (aux_loc);

		valid = gtk_tree_model_iter_next (model, iter);		
	} while (valid);

	if (go_parent) {
		iter_parent = *iter;
		if (gtk_tree_model_iter_parent (model, iter, &iter_parent) && gtk_tree_model_iter_next (model, iter)) {
			return find_location (model, iter, location, TRUE);
		}
	}
	return FALSE;
}

void
find_next_clicked (GtkButton *button)
{
	GtkTreeModel *model;
	GtkEntry *entry;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeIter iter_parent;
	GtkTreePath *path;
	const gchar *location;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(tree));
	entry = GTK_ENTRY (glade_xml_get_widget(xml, "location_search_entry"));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));

	if (gtk_tree_selection_count_selected_rows (selection) >= 1) {
		gtk_tree_selection_get_selected (selection, &model, &iter);
		/* Select next or select parent */
		if (!gtk_tree_model_iter_next (model, &iter)) {
			iter_parent = iter;
			if (!gtk_tree_model_iter_parent (model, &iter, &iter_parent) || !gtk_tree_model_iter_next (model, &iter))
				gtk_tree_model_get_iter_first (model, &iter);
		}

	} else {
		gtk_tree_model_get_iter_first (model, &iter);
	}

	location = gtk_entry_get_text (entry);

	if (find_location (model, &iter, location, TRUE)) {
		gtk_widget_set_sensitive ((GtkWidget *)button, TRUE);

		path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_view_expand_to_path (GTK_TREE_VIEW(tree), path);
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(tree), path, NULL, TRUE, 0.5, 0);

		gtk_tree_path_free (path);
	} else {
		gtk_widget_set_sensitive ((GtkWidget * )button, FALSE);
	}
}


