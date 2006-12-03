#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <itl/prayer.h>
#include <gst/gst.h>
#include "main.h"
#include "prefs.h"

/* Preferences */ 
static GConfClient     		* client;
static const GString		* pref_root_string;
static const GString 		* lat_string;
static const GString 		* lon_string;
static const GString 		* city_string;
static const GString 		* height_string;
static const GString 		* play_athan_string;

static GladeXML 		* xml;	
static gfloat 			lat;
static gfloat 			height;
static gfloat 			lon;
static gchar 			* city_name;
static GError 			* err 	= NULL;
static gchar 			* normal_time_start;
static gchar 			* normal_time_end;
static gchar 			* special_time_start;
static gchar 			* special_time_end;
static gchar			* non_prayer_start;
static gchar			* non_prayer_end;

static gchar			* default_athan_file;

/* TODO switch to using enum for these strings and structures */

int current_prayer_id = -1;

/* for prayer.h functions */
static Date 			* prayerDate;
static Location 		* loc;
static Method			* calcMethod;
static Prayer 			ptList[6];

/* For gstreamer */
static GstElement *pipeline, *source, *parser, *decoder, *conv, *sink;
static GMainLoop *loop;
static GstBus *bus;
static GtkFileFilter *filter_all;
static GtkFileFilter *filter_supported;

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
		current_prayer_id = 0;
		if(ptList[i].hour < curtime->tm_hour || 
		  	(ptList[i].hour == curtime->tm_hour && 
		   	ptList[i].minute <= curtime->tm_min))
		{
			return;
		}
	}
	
	/* time is after isha, so set to  for the day */
	current_prayer_id = 0;
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
	timestring = g_malloc(50);
	timelabel = g_malloc(20);

	gchar * markupstart;
	gchar * markupend;
	int i;
	for (i=0; i < 6; i++)
	{
		if( i == 1)
		{
			markupstart = non_prayer_start;
			markupend = non_prayer_end;
		}
		else if ( i == current_prayer_id)
		{
			markupstart = special_time_start;
			markupend = special_time_end;
		}
		else
		{
			markupstart = normal_time_start;
			markupend = normal_time_end;
		}

		g_snprintf(timelabel, 20, "salatlabel%d", i);

		g_snprintf(timestring, 50, "%s%02d:%02d%s", 
		markupstart, ptList[i].hour, 
		ptList[i].minute, markupend);
	
		gtk_label_set_markup(
			(GtkLabel *) glade_xml_get_widget(xml, timelabel),
				timestring);
	}	
}

/* needed pre getting preferences */
void set_app_defaults()
{
	pref_root_string 	= g_string_new("/apps/prayertimes/");
	lat_string		= g_string_new("city/lat");
	lon_string		= g_string_new("city/lon");
	city_string		= g_string_new("city/name");
	height_string		= g_string_new("city/height");
	play_athan_string	= g_string_new("athan/play");	
	normal_time_start 	= "<span color=\"blue\"><b>";
	normal_time_end 	= "</b></span>";
	special_time_start 	= "<span color=\"red\"><b>";
	special_time_end 	= "</b></span>";
	non_prayer_start	= "<span color=\"skyblue\"><b>";
	non_prayer_end		= "</b></span>";
	default_athan_file	= "/home/djihed/dev/gnome/myapps/prayer/athan.ogg";
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
			(const gchar *)default_athan_file);
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

	gconf_client_set_float(client, GPRAYER_PREFDIR PREF_CITY_LAT, lat, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gconf_client_set_float(client, GPRAYER_PREFDIR PREF_CITY_LON, lon, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gconf_client_set_string(client, GPRAYER_PREFDIR PREF_CITY_NAME, 
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
	GString * pref_string;	
	GString * gstmp;
	
	gstmp 		= g_string_new(pref_root_string->str);
	pref_string 	= g_string_append (gstmp, lat_string->str);
	lat 		= gconf_client_get_float(client, pref_string->str, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gstmp 		= g_string_new(pref_root_string->str);
	pref_string 	= g_string_append (gstmp, lon_string->str);
	lon 		= gconf_client_get_float(client, pref_string->str, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gstmp 		= g_string_new(pref_root_string->str);
	pref_string 	= g_string_append (gstmp, city_string->str);
	city_name	= gconf_client_get_string(client, pref_string->str, &err);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gstmp 		= g_string_new(pref_root_string->str);
	pref_string 	= g_string_append (gstmp, height_string->str);
	height 		= gconf_client_get_float(client, pref_string->str, &err);
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
	
	play_athan_at_prayer();
	return TRUE;
}

int main(int argc, char *argv[]) 
{
	/* Set defaults */
	set_app_defaults();

	/* init libraries */
	gtk_init(&argc, &argv);
	glade_init();
 	gconf_init(argc, argv, NULL);
	
	/* load gconf client */
	client = gconf_client_get_default();
	
	/* load the interface */
	xml = glade_xml_new(GPRAYER_GLADEDIR "gprayer.xml", NULL, NULL);
	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);
	
	/* initialize GStreamer */
	gst_init (&argc, &argv);
	
	/* Initialise preferenes and variables */	
	init_prefs();
	init_vars();

	/* calculate the time table, and update the labels */
	calculate_prayer_table();
	update_prayer_labels();

	g_timeout_add(60000, update_interval, NULL);

	/* start the event loop */
	gdk_threads_enter();
  	gtk_main();
  	gdk_threads_leave();

	return 0;
}
