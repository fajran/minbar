#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include <itl/prayer.h>
#include <itl/hijri.h>
#include <gst/gst.h>

#include <libnotify/notify.h>

#include "main.h"
#include "prefs.h"
#include "defines.h"

#define PROGRAM_NAME	"Gnome Prayer Times"
#define USE_TRAY_ICON   !(GTK_MINOR_VERSION < 9)

/* Preferences */ 
	
static gfloat 		lat;
static gfloat 		height;
static gfloat 		lon;
static gchar 		* city_name;
static gboolean 	enable_athan;
static int 		correction = 0;
static gboolean		notif;
static int		notiftime;
static int 		method;
static int 		next_prayer_id = -1;

/* for prayer.h functions */
static Date 		* prayerDate;
static Location 	* loc;
static Method		* calcMethod;
static Prayer 		ptList[6];

/* For libraries */
static GConfClient     	* client;
static GladeXML 	* xml;
static GError 		* err 	= NULL;
/* For gstreamer */
static GstElement 	*pipeline, *source, *parser, *decoder, *conv, *sink;
static GMainLoop 	*loop;
static GstBus 		*bus;
static GtkFileFilter 	*filter_all;
static GtkFileFilter 	*filter_supported;

/* tray icon */
#if USE_TRAY_ICON
static GtkStatusIcon   	* status_icon;	
#endif
static GDate		* currentDate;

sDate 			* hijri_date;

/* TODO localise */
gchar * hijri_month[13] = {"skip",
                      "Muharram", "Safar", "Rabi I", "Rabi II",
                      "Jumada I", "Jumada II", "Rajab", "Shaaban",
                      "Ramadan", "Shawwal", "Thul-Qiaadah", "Thul-Hijja"};

gchar * time_names[6] = {"Subh", "Shorook", "Dhuhr", 
			"Asr", "Maghreb", "isha'a"};

NotifyNotification * notification;

void setup_widgets();
void main_window_iconify_callback(GtkWidget *widget, 
		GdkEventWindowState *event);


inline void set_status_tooltip()
{
	gchar * tooltiptext;
	tooltiptext = g_malloc(2000);
	g_snprintf(tooltiptext, 2000, " %s:\t  %02d:%02d \n"
				 	" %s:   %02d:%02d \n"
					" %s:\t  %02d:%02d \n"
					" %s:\t  %02d:%02d \n"
					" %s:  %02d:%02d \n"
					" %s:\t  %02d:%02d"
					,
			time_names[0], ptList[0].hour, ptList[0].minute,
			time_names[1], ptList[1].hour, ptList[1].minute,
			time_names[2], ptList[2].hour, ptList[2].minute,
			time_names[3], ptList[3].hour, ptList[3].minute,
			time_names[4], ptList[4].hour, ptList[4].minute,
			time_names[5], ptList[5].hour, ptList[5].minute
		  );
	gtk_status_icon_set_tooltip(status_icon, tooltiptext);
	g_free(tooltiptext);
}
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
		/* salat is on next day (subh) after midnight */
		next_minutes += 60*24;
	}

	int difference = next_minutes - cur_minutes;
	int hours = difference / 60;
	int minutes = difference % 60;

	gchar * remainString;
	remainString = g_malloc(400);
	g_snprintf(remainString, 400, 
			"%sApproximatly %d hours and %d minutes\nleft for next prayer: %s.%s", 
			REMAIN_MARKUP_START, hours, minutes, time_names[next_prayer_id],
			REMAIN_MARKUP_END);
	gtk_label_set_markup((GtkLabel *) glade_xml_get_widget(xml, 
				"timeleftlabel"), remainString);
	g_free(remainString);
}

void update_date_label()
{
	gchar * dayString, * miladiString, * dateString;
	miladiString 	= g_malloc(200);
	dateString 	= g_malloc(500);
	dayString      = g_malloc(100);
	g_date_strftime (miladiString, 300, "%d %B %G", currentDate );
	g_date_strftime (dayString, 100, "%A", currentDate );

	hijri_date 	= g_malloc(sizeof(sDate));
	h_date(hijri_date, prayerDate->day, prayerDate->month, prayerDate->year);
	g_snprintf(dateString, 500, "%s%s %d %s %d \n %s%s", DATE_MARKUP_START, 
		dayString, hijri_date->day, hijri_month[hijri_date->month], 
		hijri_date->year, miladiString, DATE_MARKUP_END);

	gtk_label_set_markup((GtkLabel *)glade_xml_get_widget(xml, 
				"currentdatelabel"), dateString);
	g_free(dateString);
	g_free(miladiString);	
	g_free(dayString);	
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
		
		if ((cur_minutes + notiftime == pt_minutes ) && notif)
		{
			gchar * message;
			message = g_malloc(400);
			g_snprintf(message, 400, "%d minutes left for %s prayer.", 
					notiftime, time_names[i]); 
			show_notification(message);
			g_free(message);
		}
		if (cur_minutes == pt_minutes)
		{
			if(enable_athan){play_athan_callback();}
			if(notif)
			{
				gchar * message;
				message = g_malloc(400);
				g_snprintf(message, 400, "It is time for %s prayer.", time_names[i]); 
				show_notification(message);
				g_free(message);
			}

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


void update_prayer_labels()
{
	/* getting labels and putting time strings */
	gchar * timestring;
	gchar * timelabel;
	timestring 	= g_malloc(50);
	timelabel	= g_malloc(20);

	int i;
	for (i=0; i < 6; i++)
	{
		g_snprintf(timelabel, 20, "salatlabel%d", i);
		if( i == 1)
		{
			g_snprintf(timestring, 50, "%s%02d:%02d%s", 
				MARKUP_FAINT_START, ptList[i].hour, 
				ptList[i].minute, MARKUP_FAINT_END);
		}
		else if ( i == next_prayer_id)
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
		
	/* set UI vars */
	gtk_file_chooser_set_filename  ((GtkFileChooser *) 
			(glade_xml_get_widget(xml, "selectathan")),
			(const gchar *) GPRAYER_ATHAN_DIR"/"GPRAYER_DEFAULT_ATHAN);
	setup_file_filters();
	gtk_file_chooser_add_filter ((GtkFileChooser *) 
			(glade_xml_get_widget(xml, "selectathan")),
	       		filter_supported);

	gtk_file_chooser_add_filter ((GtkFileChooser *) 
			(glade_xml_get_widget(xml, "selectathan")),
	       		filter_all);
	update_date();

	/* Location variables */
	loc->degreeLat 		= lat;
	loc->degreeLong 	= lon;
	loc->gmtDiff 		= correction;
	loc->dst 		= 0;
	loc->seaLevel 		= 0;
	loc->pressure 		= 1010;
	loc->temperature	= 10;

	/* Calculation method */
	/* 5 is muslim world league */

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

	gconf_client_set_bool(client, PREF_PREF_PLAY, 
				enable_athan, &err);
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

	gconf_client_set_bool(client, PREF_PREF_PLAY, 
				enable_athan, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
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
	correction 	=  (int)gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget 	= glade_xml_get_widget( xml, "yesnotif");
	notif 		=  gtk_toggle_button_get_active((GtkToggleButton *)entrywidget);

	entrywidget 	= glade_xml_get_widget( xml, "notiftime");
	notiftime 	=  (int)gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget 	= glade_xml_get_widget( xml, "methodcombo");
	method 		=  (int)gtk_combo_box_get_active((GtkComboBox *)entrywidget)  + 1;
	getMethod(method, calcMethod);
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

	gconf_client_set_int(client, PREF_CITY_CORRECTION, 
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

	/* Now hide the cityedit dialog */
	gtk_widget_hide(glade_xml_get_widget( xml, "editcity"));

	/* And set the city string in the main window */
	gtk_label_set_text((GtkLabel *)
			(glade_xml_get_widget(xml, "locationname"))
			,(const gchar *)city_name);

	/* Now calculate new timetable */
	calculate_prayer_table();
	/* And set the new labels */
	update_prayer_labels();
}


void init_prefs ()
{
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
	correction  = gconf_client_get_int(client, PREF_CITY_CORRECTION, &err);
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
	if( method < 0 || method > 7)
	{
		g_printerr("Invalid calculation method in preferences, using 5: Muslim world League \n");
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

	/* And set the city string in the main window */
	gtk_label_set_text((GtkLabel *)
			(glade_xml_get_widget(xml, "locationname")),
		       	(const gchar *)city_name);
}

void play_athan_callback()
{
	
	/* Stop previously played file */
	stop_athan_callback();
	int returned = init_pipelines();
	if(returned < 0)
	{
		exit(-1);
	}
 
	/* set filename property on the file source. Also add a message
	 * handler. */
	gchar * athanfilename  = gtk_file_chooser_get_filename  
		((GtkFileChooser *) (glade_xml_get_widget(xml, "selectathan")));

	g_object_set (G_OBJECT (source), "location", athanfilename, NULL);

	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
	gst_bus_add_watch (bus, bus_call, loop);
	gst_object_unref (bus);

	/* put all elements in a bin */
	gst_bin_add_many (GST_BIN (pipeline),
		    source, parser, decoder, conv, sink, NULL);

	/* link together - note that we cannot link the parser and
	 * decoder yet, becuse the parser uses dynamic pads. For that,
	 * we set a pad-added signal handler. */
	gst_element_link (source, parser);
	gst_element_link_many (decoder, conv, sink, NULL);
	g_signal_connect (parser, "pad-added", G_CALLBACK (new_pad), NULL);
	
	/* Now set to playing and iterate. */
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
}

void stop_athan_callback()
{
	/* clean up nicely */
	if(GST_IS_ELEMENT (pipeline))
	{		
		gst_element_set_state (pipeline, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (pipeline));
	}
}

gboolean bus_call (GstBus     *bus,
	  GstMessage *msg,
	  gpointer    data)
{
	switch (GST_MESSAGE_TYPE (msg)) {
		case GST_MESSAGE_EOS:
			/* End of Stream */
			break;
		case GST_MESSAGE_ERROR: {
			gchar *debug;
			GError *err;

			gst_message_parse_error (msg, &err, &debug);
			g_free (debug);

			g_print ("Error: %s\n", err->message);
			g_error_free (err);

			set_file_status(FALSE);
			break;
		}
		default:
			set_file_status(TRUE);
			break;
		}
	return TRUE;
}

void set_file_status(gboolean status)
{
	if(status)
	{
		gtk_image_set_from_stock((GtkImage *)
			(glade_xml_get_widget(xml, "filestatusimage"))
			, GTK_STOCK_APPLY, GTK_ICON_SIZE_BUTTON);

	}
	else
	{
		gtk_image_set_from_stock((GtkImage *)
			(glade_xml_get_widget(xml, "filestatusimage"))
			, GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_BUTTON);
	}
}

void new_pad (GstElement *element,
	 	GstPad     *pad,
	 	gpointer    data)
{
	GstPad *sinkpad;
	/* We can now link this pad with the audio decoder */
	sinkpad = gst_element_get_pad (decoder, "sink");
	gst_pad_link (pad, sinkpad);

	gst_object_unref (sinkpad);
}



int init_pipelines()
{
	/* create elements */
	pipeline 	= gst_pipeline_new ("audio-player");
	source 		= gst_element_factory_make ("filesrc", "file-source");
	parser 		= gst_element_factory_make ("oggdemux", "ogg-parser");
	decoder 	= gst_element_factory_make ("vorbisdec", "vorbis-decoder");
	conv 		= gst_element_factory_make ("audioconvert", "converter");
	sink 		= gst_element_factory_make ("alsasink", "alsa-output");
	if (!pipeline || !source || !parser || !decoder || !conv || !sink) {
		g_print ("One element could not be created\n");
		return -1;
	}
	return 1;
}

void setup_file_filters (void)
{
	filter_all = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter_all, "All files");
	gtk_file_filter_add_pattern (filter_all, "*");
	g_object_ref (filter_all);

	filter_supported = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter_supported,
		"Supported files");
	gtk_file_filter_add_mime_type (filter_supported, "application/ogg");
	g_object_ref (filter_supported);
}

/* Interval to update prayer times if time/date changes */
gboolean update_interval(gpointer data)
{
	update_date(); 
	calculate_prayer_table(); 
	update_prayer_labels();
	
	play_events();
	set_status_tooltip();
	
	return TRUE;
}

/* System tray icon */
/* TODO func pref */

#if USE_TRAY_ICON
void load_system_tray()
{
	status_icon 	= gtk_status_icon_new_from_file
		(GPRAYER_PIXMAPSDIR"/"GPRAYER_KAABA_ICON);
	
	g_signal_connect ((GtkStatusIcon * ) (status_icon), "popup_menu", 
			G_CALLBACK(tray_icon_right_clicked_callback) , NULL);
	g_signal_connect ((GtkStatusIcon * ) (status_icon), "activate", 
			G_CALLBACK(tray_icon_clicked_callback) , NULL);


	
}
#endif

void quit_callback ( GtkWidget *widget, gpointer data)
{
	/* TODO cleanup? prefs? */
	gtk_main_quit();
}

void tray_icon_right_clicked_callback ( GtkWidget *widget, gpointer data)
{
	GtkMenu * popup_menu = (GtkMenu * )(glade_xml_get_widget(xml, "traypopup")); 
	
	gtk_menu_set_screen (GTK_MENU (popup_menu), NULL);
	
	gtk_menu_popup (GTK_MENU (popup_menu), NULL, NULL, NULL, NULL,
			1, gtk_get_current_event_time());
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
void show_notification(gchar * message)
{
	notify_notification_update(notification,
				PROGRAM_NAME,
				message,
				GTK_STOCK_ABOUT);
	notify_notification_show(notification, NULL);
}

void create_notification()
{
	notification = notify_notification_new
                                            (PROGRAM_NAME,
                                             NULL,
                                             NULL,
					     NULL);
	notify_notification_attach_to_status_icon (notification, status_icon );
	notify_notification_set_timeout (notification, 8000);
}

/**** Main ****/
int main(int argc, char *argv[]) 
{
	/* init libraries */
	gtk_init(&argc, &argv);
	glade_init();
 	gconf_init(argc, argv, NULL);
	notify_init(PROGRAM_NAME);
	
	/* load gconf client */
	client = gconf_client_get_default();
	
	/* load the interface */
	xml = glade_xml_new(GPRAYER_GLADEDIR"/"GLADE_MAIN_INTERFACE, NULL, NULL);
	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);
	
	/* Set up some widgets and options that not stored in the glade xml */
	setup_widgets();
	
	/* System tray icon */
#if USE_TRAY_ICON
	load_system_tray();
#endif
	/* initialize GStreamer */
	gst_init (&argc, &argv);
	
	/* Initialise preferenes and variables */	
	init_prefs();
	init_vars();

	/* calculate the time table, and update the labels */
	calculate_prayer_table();
	update_prayer_labels();
	
	/* set system tray tooltip text */
	set_status_tooltip();
	
	/* Used to balloon tray notifications */
	create_notification();
	
	/* start athan playing, time updating interval */
	g_timeout_add(60000, update_interval, NULL);

	/* start the event loop */
  	gtk_main();
	return 0;
}

void setup_widgets()
{
#if USE_TRAY_ICON
	/* TODO hide on minimise	
	GtkWidget * mainwindow = glade_xml_get_widget(xml, "mainWindow");
	*/
#else
	GtkWidget * closebutton = glade_xml_get_widget(xml, "closebutton");
	gtk_widget_hide(closebutton);
#endif
}

