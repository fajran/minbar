#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include <itl/prayer.h>
#include <gst/gst.h>
#include "main.h"
#include "prefs.h"
#include "defines.h"


void tray_icon_clicked_callback ( GtkWidget *widget, gpointer data);
void tray_icon_right_clicked_callback ( GtkWidget *widget, gpointer data);

void load_system_tray();


/* Preferences */ 
	
static gfloat 			lat;
static gfloat 			height;
static gfloat 			lon;
static gchar 			* city_name;
static gboolean 		enable_athan;

int current_prayer_id = -1;

/* for prayer.h functions */
static Date 			* prayerDate;
static Location 		* loc;
static Method			* calcMethod;
static Prayer 			ptList[6];

/* For libraries */
static GConfClient     		* client;
static GladeXML 		* xml;
static GError 			* err 	= NULL;
/* For gstreamer */
static GstElement *pipeline, *source, *parser, *decoder, *conv, *sink;
static GMainLoop *loop;
static GstBus *bus;
static GtkFileFilter *filter_all;
static GtkFileFilter *filter_supported;

/* tray icon */
struct tray_icon_struct
{
	GtkWidget       * popup_menu;
	GtkStatusIcon   * status_icon;
	
	gboolean         show_notifications;
	gboolean         is_visible;
};

struct tray_icon_struct * tray_icon;

void calculate_prayer_table()
{
	/* Update the values */
	loc->degreeLat 		= lat;
	loc->degreeLong 	= lon;	
	getPrayerTimes (loc, calcMethod, prayerDate, ptList);
	current_prayer();
}

void play_athan_at_prayer()
{
	time_t 	result;
	struct 	tm * curtime;
	result 	= time(NULL);
	curtime = localtime(&result);
	
	int i;
	for (i = 0; i < 6; i++)
	{
		if ( i == 1 ) { continue ;} /* skip shorouk */
		if ( (ptList[i].hour == curtime->tm_hour && 
		      ptList[i].minute == curtime->tm_min))
		{
			play_athan_callback();
			return;
		}
	}
}

void current_prayer()
{	
	/* current time */
	time_t result;
	struct tm * curtime;
	result = time(NULL);
	curtime = localtime(&result);
	
	int i;
	for (i = 0; i < 6; i++)
	{
		if ( i == 1 ) { continue ;} /* skip shorouk */
		current_prayer_id = (i==0) ? 5 : i-1;
		current_prayer_id = (i==2) ? 0 : i-1;
		if(ptList[i].hour > curtime->tm_hour || 
		  	(ptList[i].hour == curtime->tm_hour && 
		   	ptList[i].minute >= curtime->tm_min))
		{

			return;
		}
	}
	
	/* time is after midnight and before subh, so set to Isha */

	current_prayer_id = 5;
}

void update_date()
{
	GTimeVal * curtime 	= g_malloc(sizeof(GTimeVal));
	prayerDate 		= g_malloc(sizeof(Date));

	GDate * currentDate = g_date_new();
	g_get_current_time(curtime);
	g_date_set_time_val(currentDate, curtime);
	g_free(curtime);

	/* Setting current day */
	prayerDate = g_malloc(sizeof(Date));
	prayerDate->day = g_date_get_day(currentDate);
	prayerDate->month = g_date_get_month(currentDate);
	prayerDate->year = g_date_get_year(currentDate);

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
		else if ( i == current_prayer_id)
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
	calcMethod 		= g_malloc(sizeof(Method)); 	
	
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
	loc->gmtDiff 		= 0;
	loc->dst 		= 0;
	loc->seaLevel 		= 0;
	loc->pressure 		= 1010;
	loc->temperature	= 10;

	/* Calculation method */
	/* 5 is muslim world league */
	getMethod(5, calcMethod);

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

	gconf_client_set_bool(client, PREF_ATHAN_PLAY, 
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

	gconf_client_set_bool(client, PREF_ATHAN_PLAY, 
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
	entrywidget = glade_xml_get_widget( xml, "longitude");	
	lon =  gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget = glade_xml_get_widget( xml, "latitude");
	lat =  gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget = glade_xml_get_widget( xml, "cityname");
	g_stpcpy(city_name, gtk_entry_get_text((GtkEntry *)entrywidget)); 

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
	enable_athan  = gconf_client_get_bool(client, PREF_ATHAN_PLAY, &err);
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

	/* Set the play athan check box */
	entrywidget = glade_xml_get_widget( xml, "enabledathancheck");
	gtk_toggle_button_set_active((GtkToggleButton *) entrywidget, enable_athan);

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
	pipeline = gst_pipeline_new ("audio-player");
	source = gst_element_factory_make ("filesrc", "file-source");
	parser = gst_element_factory_make ("oggdemux", "ogg-parser");
	decoder = gst_element_factory_make ("vorbisdec", "vorbis-decoder");
	conv = gst_element_factory_make ("audioconvert", "converter");
	sink = gst_element_factory_make ("alsasink", "alsa-output");
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
	
	if(enable_athan) { play_athan_at_prayer();}
	return TRUE;
}

/* System tray icon */
/* TODO func pref */
void load_system_tray()
{
	tray_icon = g_malloc(sizeof(struct tray_icon_struct));
	tray_icon->status_icon 	= gtk_status_icon_new_from_file("quran.png");
	tray_icon->show_notifications = TRUE;
	tray_icon->is_visible = TRUE;

	g_signal_connect ((GtkStatusIcon * ) (tray_icon->status_icon), "popup_menu", 
			G_CALLBACK(tray_icon_right_clicked_callback) , NULL);
	g_signal_connect ((GtkStatusIcon * ) (tray_icon->status_icon), "activate", 
			G_CALLBACK(tray_icon_clicked_callback) , NULL);

	
}

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
/* TODO tmp */
void close_callback( GtkWidget *widget,
	    gpointer data)
{
	    gtk_widget_hide(glade_xml_get_widget(xml, "mainWindow"));
}



int main(int argc, char *argv[]) 
{
	/* init libraries */
	gtk_init(&argc, &argv);
	glade_init();
 	gconf_init(argc, argv, NULL);
	
	/* load gconf client */
	client = gconf_client_get_default();
	
	/* load the interface */
	xml = glade_xml_new(GPRAYER_GLADEDIR"/"GLADE_MAIN_INTERFACE, NULL, NULL);
	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);
	
	/* System tray icon */
	load_system_tray();

	/* initialize GStreamer */
	gst_init (&argc, &argv);
	
	/* Initialise preferenes and variables */	
	init_prefs();
	init_vars();

	/* calculate the time table, and update the labels */
	calculate_prayer_table();
	update_prayer_labels();

	/* start athan playing, time updating interval */
	g_timeout_add(60000, update_interval, NULL);

	/* start the event loop */
  	gtk_main();
	return 0;
}
