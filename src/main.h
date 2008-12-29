/* main.h
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

#ifndef MAIN_H_
#define MAIN_H_

#include "config.h"


#include <gtk/gtk.h>
#include <itl/prayer.h>
#if USE_GSTREAMER
#include <gst/gst.h>
#else
#include <xine.h>
#endif



/* Functions */ 
void 	calculate_prayer_table(void);
void 	play_athan_at_prayer(void);
void	next_prayer(void);
void 	update_date(void);
void 	update_prayer_labels(Prayer * ptList, gchar * prefix, gboolean coloured);
gboolean update_interval(gpointer data);
void 	setup_file_filters (void);
void 	init_prefs (void);
void 	init_vars(void);
void 	load_system_tray(void);
void 	show_notification(gchar * message);
void 	create_notification(void);
void 	update_remaining(void);
void 	update_date_label(void);
void 	setup_widgets(void);
void 	set_status_tooltip(void);
void 	play_events(void);
void	update_calendar(void);
void	setup_locations_applet(void);
void	set_file_status(gboolean);
#if USE_GSTREAMER
/* Gstreamer */
int 	init_pipelines(void);
void 	new_pad (GstElement *element, GstPad *pad, gpointer data);
void 	set_file_status(gboolean status);
gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data);
#endif
/* Call backs */
gboolean draw_qibla(GtkWidget *widget, GdkEventExpose *event, gpointer data);
void 	stop_athan_callback(void);
void 	play_athan_callback(void);
void 	on_enabledathancheck_toggled_callback(GtkWidget *widget, gpointer user_data);
void 	tray_icon_clicked_callback ( GtkWidget *widget, gpointer data);
void 	tray_icon_right_clicked_callback ( GtkWidget *widget, gpointer data);

void 	on_enabledathanmenucheck_toggled_callback(GtkWidget *widget, gpointer data);
void 	on_notifmenucheck_toggled_callback(GtkWidget *widget, gpointer data);
void 	check_quit_callback(GtkWidget *widget, gpointer data);
void 	quit_callback(GtkWidget *widget, gpointer data);
void 	show_window_clicked_callback( GtkWidget *widget, gpointer data);
void 	close_callback(GtkWidget *widget, gpointer data);
void 	window_state_event_callback(GtkWidget *widget, GdkEventWindowState *event);
void 	on_editcityokbutton_clicked_callback(GtkWidget *widget, gpointer user_data);
void 	play_subh_athan_callback(void);
void 	play_normal_athan_callback(void);
void 	prayer_calendar_callback(void);
void	minute_label_callback(GtkWidget *widget, gpointer user_data);
void 	locationok_callback(void);
gboolean tray_destroy_cb (GtkObject *object);
void	activate_url (GtkAboutDialog *about, const gchar *link, gpointer data);

void minbar_apply_config(void);

MinbarConfig *config;


#endif

