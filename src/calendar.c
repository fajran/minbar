
#include "calendar.h"
#include "defines.h"

#include <glade/glade.h>

static GladeXML *glade_xml;
static GtkWidget *dialog = NULL;
static Prayer times[6];

typedef struct _CalendarWidgets
{
	GtkWidget *calendar;
	GtkWidget *times[6];
} CalendarWidgets;

static CalendarWidgets* widgets;

static void
update_date (void)
{
	guint * year = g_malloc(sizeof(guint));
	guint * month = g_malloc(sizeof(guint));
	guint * day = g_malloc(sizeof(guint));

	gtk_calendar_get_date((GtkCalendar*)widgets->calendar,
			year, month, day);

	Date * cDate;	
	cDate 		= g_malloc(sizeof(Date));
	cDate->day 	= (int) *day;
	cDate->month 	= (int) (*month) +1;
	cDate->year 	= (int) *year;

	getPrayerTimes (loc, calcMethod, cDate, times);

	gchar * timestring;
	timestring 	= g_malloc(50);

	int i;
	for (i=0; i<6; i++)
	{
		g_snprintf(timestring, 50, "%s%02d:%02d%s", 
				MARKUP_NORMAL_START, times[i].hour, 
				times[i].minute, MARKUP_NORMAL_END);

		gtk_label_set_markup((GtkLabel*)widgets->times[i], timestring);
	}

	g_free(timestring);
	g_free(cDate);
	g_free(year);
	g_free(month);
	g_free(day);
}

static void
response (GtkWidget *widget, gint id, gpointer data)
{
	if (id == GTK_RESPONSE_CLOSE)
	{
		gtk_widget_destroy(widget);
	}
}

static void
dialog_destroyed (GtkWidget *widget, gpointer data)
{
	dialog = NULL;
	g_free(widgets);
}

static gboolean
delete_event (GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(widget);
	return TRUE;
}

static void
on_day_selected (GtkWidget *widget, gpointer data)
{
	update_date();
}

static void
on_today_clicked (GtkWidget *widget, gpointer data)
{
	gtk_calendar_select_month((GtkCalendar *)widgets->calendar,
			prayerDate->month - 1, prayerDate->year);
	gtk_calendar_select_day((GtkCalendar *)widgets->calendar,
			prayerDate->day);
}

static void
setup_dialog (void)
{
	GladeXML *xml = glade_xml;

	// Prayer names

	gchar *labeltext;
	labeltext = g_malloc(100);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[0]);
	gtk_label_set_markup((GtkLabel*)glade_xml_get_widget(xml, "subhc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[1]);
	gtk_label_set_markup((GtkLabel*)glade_xml_get_widget(xml, "shouroukc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[2]);
	gtk_label_set_markup((GtkLabel*)glade_xml_get_widget(xml, "duhrc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[3]);
	gtk_label_set_markup((GtkLabel*)glade_xml_get_widget(xml, "asrc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[4]);
	gtk_label_set_markup((GtkLabel*)glade_xml_get_widget(xml, "maghrebc"), labeltext);

	g_snprintf(labeltext, 100, "<b>%s:</b>", time_names[5]);
	gtk_label_set_markup((GtkLabel*)glade_xml_get_widget(xml, "ishac"), labeltext);

	g_free(labeltext);

	// Widgets
	
	widgets = g_malloc(sizeof(CalendarWidgets));

	widgets->calendar = glade_xml_get_widget(xml, "prayer_calendar");

	widgets->times[0] = glade_xml_get_widget(xml, "salatlabelc0");
	widgets->times[1] = glade_xml_get_widget(xml, "salatlabelc1");
	widgets->times[2] = glade_xml_get_widget(xml, "salatlabelc2");
	widgets->times[3] = glade_xml_get_widget(xml, "salatlabelc3");
	widgets->times[4] = glade_xml_get_widget(xml, "salatlabelc4");
	widgets->times[5] = glade_xml_get_widget(xml, "salatlabelc5");

	// Signals
	
	g_signal_connect(dialog, "destroy",
		G_CALLBACK(dialog_destroyed), NULL);
	g_signal_connect(dialog, "delete_event",
		G_CALLBACK(delete_event), NULL);
	g_signal_connect(dialog, "response",
		G_CALLBACK(response), NULL);

	g_signal_connect(widgets->calendar, "day-selected",
		G_CALLBACK(on_day_selected), NULL);

	g_signal_connect(glade_xml_get_widget(xml, "calendar_today"), "clicked",
		G_CALLBACK(on_today_clicked), NULL);

}

void
calendar_show (void)
{
	if (dialog == NULL)
	{
		glade_xml = glade_xml_new(g_build_filename(MINBAR_DATADIR,GLADE_MAIN_INTERFACE,NULL), NULL, NULL);
	
		dialog = glade_xml_get_widget(glade_xml, "CalendarDialog");
		setup_dialog();
		update_date();
	}

	gtk_window_present(GTK_WINDOW(dialog));
}

void 
calendar_update (void)
{
	if (dialog != NULL)
	{
		// TODO
	}
}
