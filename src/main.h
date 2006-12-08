/* Functions */ 
void 		calculate_prayer_table();
void 		play_athan_at_prayer();
void		next_prayer();
void 		update_date();
void 		update_prayer_labels();
gboolean 	update_interval(gpointer data);
void 		setup_file_filters (void);
void 		init_prefs ();
void 		init_vars();
void 		load_system_tray();
void 		show_notification(gchar * message);
void 		create_notification();
void 		update_remaining();
void 		update_date_label();

/* Gstramer */
int 		init_pipelines();
void 		new_pad (GstElement *element, GstPad *pad, gpointer data);
void 		set_file_status(gboolean status);
gboolean 	bus_call (GstBus *bus, GstMessage *msg, gpointer data);

/* Call backs */
void 		stop_athan_callback();
void 		play_athan_callback();
void 		on_enabledathancheck_toggled_callback(GtkWidget *widget,
				gpointer user_data);
void 		tray_icon_clicked_callback ( GtkWidget *widget, gpointer data);
void 		tray_icon_right_clicked_callback ( GtkWidget *widget, gpointer data);

