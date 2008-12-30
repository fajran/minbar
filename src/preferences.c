
#include <glade/glade.h>
#include <glib/gi18n.h>

#include "main.h"
#include "defines.h"
#include "prefs.h"
#include "config.h"
#include "preferences.h"
#include "locations-xml.h"

//TODO
// #if !USE_GSTREAMER
// #include <xine.h>
// #endif

#include <stdio.h>

static GladeXML *glade_xml = NULL;

static GtkWidget *dialog = NULL;
static GtkWidget *locations_dialog = NULL;

static GtkFileFilter 	*filter_all;
static GtkFileFilter 	*filter_supported;

typedef struct _PreferencesWidgets
{
	GtkWidget *latitude;
	GtkWidget *longitude;
	GtkWidget *city;

	GtkWidget *correction;
	GtkWidget *method;

	GtkWidget *start_hidden;
	//TODO: GtkWidget *close_closes;

	GtkWidget *notification;
	GtkWidget *notification_time;

	GtkWidget *athan_enabled;
	GtkWidget *athan_subh;
	GtkWidget *athan_normal;
} PreferencesWidgets;

typedef struct _LocationsWidgets
{
	GtkWidget *tree;
	GtkWidget *next_button;
} LocationsWidgets;

static PreferencesWidgets *widgets;
static LocationsWidgets *locations_widgets;

void load_locations_callback();
void find_entry_changed (GtkEditable *entry);
void find_next_clicked (GtkButton *button);

void 
preferences_set_notification(gboolean notification)
{
	printf("set notification: %d\n", notification);
	if (dialog && widgets)
	{
		gtk_toggle_button_set_active((GtkToggleButton *)widgets->notification, notification);
	}
}

static gboolean
valid_file(const gchar* filename)
{
	FILE * testfile;
	testfile = fopen(filename, "r");
	return testfile != NULL;
}

static void 
update_config (void)
{
	config->latitude = gtk_spin_button_get_value((GtkSpinButton*)widgets->latitude);
	config->longitude = gtk_spin_button_get_value((GtkSpinButton *)widgets->longitude);
	config->city = g_strdup(gtk_entry_get_text((GtkEntry*)widgets->city));

	config->correction = gtk_spin_button_get_value((GtkSpinButton *)widgets->correction);
	config->method = gtk_combo_box_get_active((GtkComboBox *)widgets->method) + 1;

	config->start_hidden = gtk_toggle_button_get_active((GtkToggleButton *)widgets->start_hidden);
	//TODO: config->close_closes = gtk_toggle_button_get_active((GtkToggleButton *)widgets->close_closes);

	config->notification = gtk_toggle_button_get_active((GtkToggleButton *)widgets->notification);
	config->notification_time = gtk_spin_button_get_value((GtkSpinButton *)widgets->notification_time);

	config->athan_enabled = gtk_toggle_button_get_active((GtkToggleButton *) widgets->athan_enabled);
	config->athan_subh = gtk_file_chooser_get_filename  ((GtkFileChooser*)widgets->athan_subh);
	config->athan_normal = gtk_file_chooser_get_filename  ((GtkFileChooser*)widgets->athan_normal);
}

static void
update_widgets (void)
{
	gtk_spin_button_set_value((GtkSpinButton *)widgets->latitude, config->latitude);
	gtk_spin_button_set_value((GtkSpinButton *)widgets->longitude, config->longitude);
	gtk_entry_set_text((GtkEntry *)widgets->city, config->city);

	gtk_spin_button_set_value((GtkSpinButton *)widgets->correction, config->correction);
	gtk_combo_box_set_active((GtkComboBox *)widgets->method, config->method-1);

	gtk_toggle_button_set_active((GtkToggleButton *)widgets->notification, config->notification);
	gtk_spin_button_set_value((GtkSpinButton *)widgets->notification_time, config->notification_time);

	gtk_toggle_button_set_active((GtkToggleButton *)widgets->start_hidden, config->start_hidden);
	//TODO: gtk_toggle_button_set_active((GtkToggleButton *)widgets->close_closes, config->close_closes);

	gtk_toggle_button_set_active((GtkToggleButton *) widgets->athan_enabled, config->athan_enabled);
	if (valid_file(config->athan_subh))
	{
		gtk_file_chooser_set_filename  ((GtkFileChooser*)widgets->athan_subh, (const gchar *) config->athan_subh);
	}
	if (valid_file(config->athan_normal))
	{
		gtk_file_chooser_set_filename  ((GtkFileChooser*)widgets->athan_normal,
			(const gchar *) config->athan_normal);
	}
}

static void
setup_file_filters_fixme (void)
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
	xine_t		*xine;
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

static void
response (GtkWidget *widget, gint id, gpointer data)
{
	if (id == GTK_RESPONSE_OK)
	{
		update_config();
		config_save(config);
		minbar_apply_config();
	}

	gtk_widget_destroy(widget);
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
setup_dialog()
{
	GladeXML *xml = glade_xml;

	g_signal_connect(dialog, "destroy",
		G_CALLBACK(dialog_destroyed), NULL);
	g_signal_connect(dialog, "delete_event",
		G_CALLBACK(delete_event), NULL);
	g_signal_connect(dialog, "response",
		G_CALLBACK(response), NULL);
	
	widgets = g_malloc(sizeof(PreferencesWidgets));
	
	widgets->latitude = glade_xml_get_widget( xml, "latitude");	
	widgets->longitude = glade_xml_get_widget( xml, "longitude");	
	widgets->city = glade_xml_get_widget( xml, "cityname");	

	widgets->correction = glade_xml_get_widget( xml, "correction");	
	widgets->method = glade_xml_get_widget( xml, "methodcombo");	

	widgets->notification = glade_xml_get_widget( xml, "yesnotif");	
	widgets->notification_time = glade_xml_get_widget( xml, "notiftime");	
	widgets->start_hidden = glade_xml_get_widget( xml, "startHidden");

	widgets->athan_enabled = glade_xml_get_widget( xml, "enabledathancheck");
	widgets->athan_subh = glade_xml_get_widget(xml, "athan_subh_chooser");
	widgets->athan_normal = glade_xml_get_widget(xml, "athan_chooser");

	setup_file_filters_fixme();
	gtk_file_chooser_add_filter ((GtkFileChooser*)widgets->athan_subh,
	       		filter_supported);
	gtk_file_chooser_add_filter ((GtkFileChooser*)widgets->athan_subh,
	       		filter_all);

	setup_file_filters_fixme();
	gtk_file_chooser_add_filter ((GtkFileChooser*)widgets->athan_normal,
	       		filter_supported);
	gtk_file_chooser_add_filter ((GtkFileChooser*)widgets->athan_normal,
	       		filter_all);
}



void 
preferences_show (void)
{
	if (dialog == NULL) 
	{
		glade_xml = glade_xml_new(g_build_filename(MINBAR_DATADIR,GLADE_MAIN_INTERFACE,NULL), NULL, NULL);
		glade_xml_signal_autoconnect(glade_xml);

		dialog = glade_xml_get_widget(glade_xml, "editcity");

		setup_dialog();
		update_widgets();
	}
	gtk_window_present(GTK_WINDOW(dialog));
}

////////////////////////////////////////////////////////////////////////////////////

static int
locations_apply()
{
	if(!locations_widgets->tree)
		return 0;

	GtkTreeSelection * selection;
	selection = gtk_tree_view_get_selection ((GtkTreeView *)locations_widgets->tree);
		
	GtkTreeModel *model;	
	GtkTreeIter iter;

	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return 0;

	WeatherLocation *loc = NULL;

	gtk_tree_model_get (model, &iter, GWEATHER_XML_COL_POINTER, &loc, -1);

	if (!loc)
		return 0;

	/*g_print("%s, %f,%f \n", loc->name, loc->longitude, loc->latitude);*/

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->latitude), (loc->latitude * 180) / M_PI);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->longitude), (loc->longitude * 180) / M_PI);
	gtk_entry_set_text(GTK_ENTRY(widgets->city), loc->name);

	return 1;

}

static void
locations_response (GtkWidget *widget, gint id, gpointer data)
{
	if (id == GTK_RESPONSE_APPLY)
	{
		if (!locations_apply())
		{
			return;
		}
	}

	gtk_widget_hide(widget);
}

static gboolean
locations_delete_event (GtkWidget *widget, gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}

static void 
setup_locations_dialog()
{
	GladeXML *xml = glade_xml;

	g_signal_connect(locations_dialog, "delete_event",
		G_CALLBACK(locations_delete_event), NULL);
	g_signal_connect(locations_dialog, "response",
		G_CALLBACK(locations_response), NULL);

	GtkTreeStore *model;
	/*GtkTreeSelection *selection;*/
	GtkWidget *scrolled_window;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell_renderer;
	/*WeatherLocation *current_location;*/

       	scrolled_window	= (GtkWidget*) glade_xml_get_widget(xml, "location_list_scroll");

	model 		= gtk_tree_store_new (GWEATHER_XML_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
	locations_widgets->tree 		= gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (locations_widgets->tree), FALSE);

	/* Add a colum for the locations */
	cell_renderer 	= gtk_cell_renderer_text_new ();
	column 		= gtk_tree_view_column_new_with_attributes ("not used", cell_renderer,
				       "text", GWEATHER_XML_COL_LOC, NULL);
	gtk_tree_view_append_column ((GtkTreeView *)locations_widgets->tree, column);
	gtk_tree_view_set_expander_column (GTK_TREE_VIEW (locations_widgets->tree), column);

	gtk_container_add (GTK_CONTAINER (scrolled_window), locations_widgets->tree);
	gtk_widget_show (locations_widgets->tree);
	gtk_widget_show (scrolled_window);

	/* current_location = weather_location_clone (gw_applet->gweather_pref.location);*/ 
	/* load locations from xml file */
	if (gweather_xml_load_locations ((GtkTreeView *)locations_widgets->tree, NULL))
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

void 
load_locations_callback()
{
	if (locations_dialog == NULL)
	{
		locations_widgets = g_malloc(sizeof(LocationsWidgets));
		locations_dialog = glade_xml_get_widget(glade_xml, "locationsDialog");
		setup_locations_dialog();
	}
	gtk_dialog_run(GTK_DIALOG(locations_dialog));
}

/* shamelessly copied from gweather code */
static gboolean
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
find_entry_changed (GtkEditable *entry/*, GWeatherPref *pref*/)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkWidget *nextbutton;
	const gchar *location;

	nextbutton = (GtkWidget *) glade_xml_get_widget(glade_xml, "findnextbutton");

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(locations_widgets->tree));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (locations_widgets->tree));
	gtk_tree_model_get_iter_first (model, &iter);

	location = gtk_entry_get_text (GTK_ENTRY (entry));

	if (find_location (model, &iter, location, TRUE)) 
	{
		gtk_widget_set_sensitive (nextbutton , TRUE);

		path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_view_expand_to_path (GTK_TREE_VIEW(locations_widgets->tree), path);
		gtk_tree_selection_select_iter (selection, &iter);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(locations_widgets->tree), path, NULL, TRUE, 0.5, 0);

		gtk_tree_path_free (path);
	} 
	else 
	{
		gtk_widget_set_sensitive (nextbutton, FALSE);
	}
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

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(locations_widgets->tree));
	entry = GTK_ENTRY (glade_xml_get_widget(glade_xml, "location_search_entry"));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (locations_widgets->tree));

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
		gtk_tree_view_expand_to_path (GTK_TREE_VIEW(locations_widgets->tree), path);
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(locations_widgets->tree), path, NULL, TRUE, 0.5, 0);

		gtk_tree_path_free (path);
	} else {
		gtk_widget_set_sensitive ((GtkWidget * )button, FALSE);
	}
}





