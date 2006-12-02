
#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <string.h>
#include <glib.h>
#include <itl/prayer.h>
#include <gst/gst.h>
#include <unistd.h>

void on_editcityokbutton_clicked(GtkWidget *widget, gpointer user_data); 
void calculate_prayer_table();
void play_athan_callback();
void stop_athan_callback();
static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data);
static void new_pad (GstElement *element, GstPad *pad, gpointer data);
int init_pipelines();
void setup_file_filters (void);
void set_file_status(gboolean status);
void update_prayer_labels();
void lock_variables();
void unlock_variables();

static GConfClient     		* client;
static const GString		* prefRootString;
static const GString 		* latstring;
static const GString 		* lonstring;
static const GString 		* citystring;
static const GString 		* heightstring;
static GladeXML 		* xml;	
static gfloat 			lat;
static gfloat 			height;
static gfloat 			lon;
static gchar 			* cityname;
static GError 			* err 	= NULL;
static gchar 			* normal_time_start;
static gchar 			* normal_time_end;
static gchar 			* special_time_start;
static gchar 			* special_time_end;
static gchar			* defaultAthanFile;

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


}

void update_prayer_labels()
{
	/* getting labels and putting time strings */
	gchar * timestring;
	timestring = g_malloc(50);

	g_snprintf(timestring, 50, "%s%02d:%02d%s", 
		normal_time_start, ptList[0].hour, 
		ptList[0].minute, normal_time_end);
	
	
	g_print("%s\n", timestring);
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "subhtime"),
		timestring);

	g_snprintf(timestring, 50, "%s%02d:%02d%s",  
			normal_time_start, ptList[1].hour,
			ptList[1].minute, normal_time_end); 
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "shorooktime"),
		timestring);

	g_snprintf(timestring, 50, "%s%02d:%02d%s",  
			normal_time_start, ptList[2].hour,
			ptList[2].minute, normal_time_end); 
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "duhrtime"),
		timestring);

	g_snprintf(timestring, 50, "%s%02d:%02d%s",  
			normal_time_start, ptList[3].hour,
			ptList[3].minute, normal_time_end); 
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "asrtime"),
		timestring);
	g_snprintf(timestring, 50, "%s%02d:%02d%s",  
			normal_time_start, ptList[4].hour,
			ptList[4].minute, normal_time_end); 
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "maghrebtime"),
		timestring);

	g_snprintf(timestring, 50, "%s%02d:%02d%s",  
			normal_time_start, ptList[5].hour,
			ptList[5].minute, normal_time_end); 
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "ishatime"),
		timestring);
}

/* needed pre gettting preferences */
void setDefaults()
{
	prefRootString 	= g_string_new("/apps/prayertimes/");
	latstring	= g_string_new("city/lat");
	lonstring	= g_string_new("city/lon");
	citystring	= g_string_new("city/name");
	heightstring	= g_string_new("city/height");
	normal_time_start 	= "<span color=\"red\"><b>";
	normal_time_end 	= "</b></span>";
	special_time_start 	= "<span color=\"green\"><b>";
	special_time_end 	= "</b></span>";
	defaultAthanFile	= "/home/djihed/dev/gnome/myapps/prayer/athan.ogg";
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
	g_print("%d, %d, %d \n", g_date_get_day(currentDate),
				g_date_get_month(currentDate),
			       	g_date_get_year(currentDate));
	g_free(currentDate);
}
/* needed post loading preferences.*/
void setVars()
{
	/* Allocate memory for variables */
		loc 			= g_malloc(sizeof(Location));
	calcMethod 		= g_malloc(sizeof(Method)); 	
	
	/* set UI vars */
	gtk_file_chooser_set_filename  ((GtkFileChooser *) (glade_xml_get_widget(xml, "selectathan")),
			(const gchar *)defaultAthanFile);
	setup_file_filters();
	gtk_file_chooser_add_filter ((GtkFileChooser *) (glade_xml_get_widget(xml, "selectathan")),
	       	filter_supported);

	gtk_file_chooser_add_filter ((GtkFileChooser *) (glade_xml_get_widget(xml, "selectathan")),
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



void on_editcityokbutton_clicked(GtkWidget *widget, gpointer user_data) {

	GtkWidget*  entrywidget;	
	/* Setting what was found to editcity dialog*/
	entrywidget = glade_xml_get_widget( xml, "longitude");	
	lon =  gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget = glade_xml_get_widget( xml, "latitude");
	lat =  gtk_spin_button_get_value((GtkSpinButton *)entrywidget);

	entrywidget = glade_xml_get_widget( xml, "cityname");
	g_stpcpy(cityname, gtk_entry_get_text((GtkEntry *)entrywidget)); 

        /* set gconf settings */
	gboolean success = FALSE;
	GString * prefString;	
	GString * gstmp;

	gstmp           = g_string_new(prefRootString->str);
	prefString      = g_string_append (gstmp, latstring->str);
	success 	= gconf_client_set_float(client, prefString->str, lat, &err);
	g_print("%s\n", gstmp->str);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gstmp           = g_string_new(prefRootString->str);
	prefString      = g_string_append (gstmp, lonstring->str);
	success 	= gconf_client_set_float(client, prefString->str, lon, &err);
	g_print("%s\n", gstmp->str);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	gstmp           = g_string_new(prefRootString->str);
	prefString      = g_string_append (gstmp, citystring->str);
	success 	= gconf_client_set_string(client, 
				prefString->str, cityname, &err);
	g_print("%s\n", gstmp->str);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}

	/* Now hide the cityedit dialog */
	gtk_widget_hide(glade_xml_get_widget( xml, "editcity"));

	/* And set the city string in the main window */
	gtk_label_set_text((GtkLabel *)(glade_xml_get_widget(xml, "locationname"))
			, (const gchar *)cityname);

	/* Now calculate new timetable */
	calculate_prayer_table();
	/* And set the new labels */
	update_prayer_labels();
}


void init_prefs ()
{
	

	/*gdouble defaultlat = 21.4200;
	gdouble defaultlon = 39.8300;
	gdouble defaultheight = 0;
	gchar * defaultcity = "Makkah";*/
	
	GString * prefString;	
	GString * gstmp;
	
	
	gstmp 		= g_string_new(prefRootString->str);
	prefString 	= g_string_append (gstmp, latstring->str);
	lat 		= gconf_client_get_float(client, prefString->str, &err);
	g_print("%s\n", gstmp->str);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gstmp 		= g_string_new(prefRootString->str);
	prefString 	= g_string_append (gstmp, lonstring->str);
	lon 		= gconf_client_get_float(client, prefString->str, &err);
	g_print("%s\n", gstmp->str);	
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gstmp 		= g_string_new(prefRootString->str);
	prefString 	= g_string_append (gstmp, citystring->str);
	cityname	= gconf_client_get_string(client, prefString->str, &err);
	g_print("%s\n", gstmp->str);
	if(err != NULL)
	{
		g_print("%s\n", err->message);
		err = NULL;
	}
	gstmp 		= g_string_new(prefRootString->str);
	prefString 	= g_string_append (gstmp, heightstring->str);
	height 		= gconf_client_get_float(client, prefString->str, &err);
	g_print("%s\n", gstmp->str);
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
	gtk_entry_set_text((GtkEntry *)entrywidget, cityname);

		
	g_print("%f\n", lat);
	g_print("%f\n", lon);
	g_print("%s\n", cityname);

	/* And set the city string in the main window */
	gtk_label_set_text((GtkLabel *)(glade_xml_get_widget(xml, "locationname")),
		       	(const gchar *)cityname);
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
	g_print ("Setting to PLAYING\n");
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
	g_print ("Running\n");
}

void stop_athan_callback()
{
	/* clean up nicely */
	g_print ("Returned, stopping playback\n");
	
	if(GST_IS_ELEMENT (pipeline))
	{		
		gst_element_set_state (pipeline, GST_STATE_NULL);
		g_print ("Deleting pipeline\n");
		gst_object_unref (GST_OBJECT (pipeline));
	}
	
	
}

static gboolean
bus_call (GstBus     *bus,
	  GstMessage *msg,
	  gpointer    data)
{
	switch (GST_MESSAGE_TYPE (msg)) {
		case GST_MESSAGE_EOS:
			g_print ("End-of-stream\n");
			break;
		case GST_MESSAGE_ERROR: {
			gchar *debug;
			GError *err;

			gst_message_parse_error (msg, &err, &debug);
			g_free (debug);

			g_print ("Errorrr: %s\n", err->message);
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
		gtk_image_set_from_stock((GtkImage *)(glade_xml_get_widget(xml, "filestatusimage"))
			, GTK_STOCK_APPLY, GTK_ICON_SIZE_BUTTON);

	}
	else
	{
		gtk_image_set_from_stock((GtkImage *)(glade_xml_get_widget(xml, "filestatusimage"))
			, GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_BUTTON);
	}
}

void new_pad (GstElement *element,
	 	GstPad     *pad,
	 	gpointer    data)
{
	GstPad *sinkpad;
	/* We can now link this pad with the audio decoder */
	g_print ("Dynamic pad created, linking parser/decoder\n");

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


/* Thread to update date variable .. */
void *watch_athan_thread(void *args)
{
	for(;;)
	{
		/* sleep a while */
		sleep(10);
	
		g_print("Thread looping...\n");
		lock_variables();	
		update_date();	
		calculate_prayer_table();
		unlock_variables();

		gdk_threads_enter();
		update_prayer_labels();
		gdk_flush ();
		gdk_threads_leave();
	}
}


void lock_variables()
{
}

void unlock_variables()
{
}


int main(int argc, char *argv[]) 
{
	/* Set defaults */
	setDefaults();

	
	g_thread_init(NULL);
	gdk_threads_init();


	/* init libraries */
	gtk_init(&argc, &argv);
	glade_init();
 	gconf_init(argc, argv, NULL);
	
	/* load gconf client */
	client = gconf_client_get_default();
	
	/* load the interface */
	xml = glade_xml_new("gprayer.xml", NULL, NULL);
	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);
	
	/* initialize GStreamer */
	gst_init (&argc, &argv);
	
	/* Initialise preferenes and variables */	
	init_prefs();
	setVars();

	/* calculate the time table, and update the labels */
	calculate_prayer_table();
	update_prayer_labels();
	

	/* init threads */
  	if (!g_thread_create(watch_athan_thread, NULL, FALSE, &err))
    	{
      		g_printerr ("Failed to create thread: %s\n", err->message);
      		return 1;
    	}
	
	/* start the event loop */
	gdk_threads_enter();
  	gtk_main();
  	gdk_threads_leave();

	return 0;
}
