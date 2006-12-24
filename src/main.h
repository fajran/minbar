/* Functions */ 
void 		calculate_prayer_table(void);
void 		play_athan_at_prayer(void);
void		next_prayer(void);
void 		update_date(void);
void 		update_prayer_labels(void);
gboolean 	update_interval(gpointer data);
void 		setup_file_filters (void);
void 		init_prefs (void);
void 		init_vars(void);
void 		load_system_tray(void);
void 		show_notification(gchar * message);
void 		create_notification(void);
void 		update_remaining(void);
void 		update_date_label(void);
void 		setup_widgets(void);
void 		set_status_tooltip(void);
void 		play_events(void);

/* Gstramer */
int 		init_pipelines(void);
void 		new_pad (GstElement *element, GstPad *pad, gpointer data);
void 		set_file_status(gboolean status);
gboolean 	bus_call (GstBus *bus, GstMessage *msg, gpointer data);

/* Call backs */
void 		stop_athan_callback(void);
void 		play_athan_callback(void);
void 		on_enabledathancheck_toggled_callback(GtkWidget *widget,
				gpointer user_data);
void 		tray_icon_clicked_callback ( GtkWidget *widget, gpointer data);
void 		tray_icon_right_clicked_callback ( GtkWidget *widget, gpointer data);

void on_enabledathanmenucheck_toggled_callback(GtkWidget *widget, gpointer data);
void on_notifmenucheck_toggled_callback(GtkWidget *widget, gpointer data);
void check_quit_callback(GtkWidget *widget, gpointer data);
void quit_callback(GtkWidget *widget, gpointer data);
void show_window_clicked_callback( GtkWidget *widget, gpointer data);
void close_callback(GtkWidget *widget, gpointer data);
void window_state_event_callback(GtkWidget *widget, GdkEventWindowState *event);
void on_editcityokbutton_clicked_callback(GtkWidget *widget, gpointer user_data);




