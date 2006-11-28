
#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <string.h>
#include <glib.h>
#include <itl/prayer.h>
#include <libgnome/libgnome.h>

void on_editcityokbutton_clicked(GtkWidget *widget, gpointer user_data); 
void calculate_prayer_table();
void play_athan_callback();
void stop_athan_callback();

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


/* for prayer.h functions */
static Date 			* prayerDate;
static Location 		* loc;
static Method			* calcMethod;
static Prayer 			ptList[6];

void calculate_prayer_table()
{
	/* Update the values */
	loc->degreeLat 		= lat;
	loc->degreeLong 	= lon;
	
	getPrayerTimes (loc, calcMethod, prayerDate, ptList);

	/* getting labels and putting time strings */
	gchar * timestring;
	timestring = g_malloc(50);

	g_snprintf(timestring, 50, "%s%d:%d%s", 
		normal_time_start, ptList[0].hour, 
		ptList[0].minute, normal_time_end);
	
	
	g_print("%s\n", timestring);
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "subhtime"),
		timestring);

	g_snprintf(timestring, 50, "%s%d:%d%s",  
			normal_time_start, ptList[1].hour,
			ptList[1].minute, normal_time_end); 
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "shorooktime"),
		timestring);

	g_snprintf(timestring, 50, "%s%d:%d%s",  
			normal_time_start, ptList[2].hour,
			ptList[2].minute, normal_time_end); 
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "duhrtime"),
		timestring);

	g_snprintf(timestring, 50, "%s%d:%d%s",  
			normal_time_start, ptList[3].hour,
			ptList[3].minute, normal_time_end); 
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "asrtime"),
		timestring);
	g_snprintf(timestring, 50, "%s%d:%d%s",  
			normal_time_start, ptList[4].hour,
			ptList[4].minute, normal_time_end); 
	gtk_label_set_markup(
		(GtkLabel *) glade_xml_get_widget(xml, "maghrebtime"),
		timestring);

	g_snprintf(timestring, 50, "%s%d:%d%s",  
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
	normal_time_start = "<span color=\"red\"><b>";
	normal_time_end = "</b></span>";
	special_time_start = "<span color=\"green\"><b>";
	special_time_end = "</b></span>";
}

/* needed post setting preferences.*/
void setVars()
{
	/* Allocate memory for variables */
	GTimeVal * curtime 	= g_malloc(sizeof(GTimeVal));
	prayerDate 		= g_malloc(sizeof(Date));
	loc 			= g_malloc(sizeof(Location));
	calcMethod 		= g_malloc(sizeof(Method)); 	
	
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

	calculate_prayer_table();
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

int main(int argc, char *argv[]) 
{
	/* Set defaults */
	setDefaults();

	/* init libraries */
	gtk_init(&argc, &argv);
	glade_init();
 	gconf_init(argc, argv, NULL);
	/*libgnome_init();*/

	/* load gconf client */
	client = gconf_client_get_default();
	
	/* load the interface */
	xml = glade_xml_new("gprayer.xml", NULL, NULL);
	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);
	
	/* Initialise preferenes and variables */	
	init_prefs();
	setVars();

	/* start the event loop */
	gtk_main();
	return 0;
}

void play_athan_callback()
{
}

void stop_athan_callback()
{
}
