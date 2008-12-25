#include "config.h"

static GError *err = NULL;

#if USE_GCONF
static GConfClient *data;
#else
static GKeyFile *data;
#endif

static MinbarConfig* 
config_new(void) 
{
	MinbarConfig* config = g_malloc(sizeof(MinbarConfig));
	
	// TODO: initialize variables

	return config;
}

void 
config_init(void) 
{
#if USE_GCONF
	data = gconf_client_get_default();
#else
	data = g_key_file_new();
	gchar* filename = g_build_filename(g_get_user_config_dir(), "minbar", "prefs.conf", NULL);
	g_key_file_load_from_file(data, filename, G_KEY_FILE_NONE, &err);
	g_free(filename);
#endif
}

MinbarConfig* config_read(void) {
	MinbarConfig* config = config_new();
#if USE_GCONF

	gconf_client_clear_cache(data);

	config->city = gconf_client_get_string(data, PREF_CITY_NAME, &err);
	config->latitude = gconf_client_get_float(data, PREF_CITY_LAT, &err);
	config->longitude = gconf_client_get_float(data, PREF_CITY_LON, &err);

	config->correction = gconf_client_get_float(data, PREF_CITY_CORRECTION, &err);
	config->method = gconf_client_get_int(data, PREF_PREF_METHOD, &err);

	config->start_hidden = gconf_client_get_bool(data, PREF_PREF_HIDDEN, &err);
	config->close_closes = gconf_client_get_bool(data, PREF_PREF_CLOSES, &err);

	config->notification = gconf_client_get_bool(data, PREF_PREF_NOTIF, &err);
	config->notification_time = gconf_client_get_int(data, PREF_PREF_NOTIF_TIME, &err);

	config->athan_enabled = gconf_client_get_bool(data, PREF_PREF_PLAY, &err);
	config->athan_normal = gconf_client_get_string(data, PREF_ATHAN_NORMAL, &err);
	config->athan_subh = gconf_client_get_string(data, PREF_ATHAN_SUBH, &err);

#else
	config->city = g_key_file_get_string(data, "city", "name", &err);
	config->latitude = g_key_file_get_double(data, "city", "latitude", &err);
	config->longitude = g_key_file_get_double(data, "city", "longitude", &err);

	config->correction = g_key_file_get_double(data, "prefs", "correction", &err);
	config->method = g_key_file_get_integer(data, "prefs", "method", &err);

	config->start_hidden = g_key_file_get_boolean(data, "prefs", "starthidden", &err);
	config->close_closes = g_key_file_get_boolean(data, "prefs", "closes", &err);

	config->notification = g_key_file_get_boolean(data, "prefs", "notif", &err);
	config->notification_time = g_key_file_get_integer(data, "prefs", "notiftime", &err);

	config->athan_enabled = g_key_file_get_boolean(data, "prefs", "play", &err);
	config->athan_normal = g_key_file_get_string(data, "athan", "normal", &err);
	config->athan_subh = g_key_file_get_string(data, "athan", "subh", &err);
#endif

	return config;
}

void config_save(MinbarConfig* config) {
#if USE_GCONF
	gconf_client_set_string(data, PREF_CITY_NAME, config->city, &err);
	gconf_client_set_float(data, PREF_CITY_LAT, config->latitude, &err);
	gconf_client_set_float(data, PREF_CITY_LON, config->longitude, &err);
	
	gconf_client_set_float(data, PREF_CITY_CORRECTION, config->correction, &err);
	gconf_client_set_int(data, PREF_PREF_METHOD, config->method, &err);
	
	gconf_client_set_bool(data, PREF_PREF_HIDDEN, config->start_hidden, &err);
	gconf_client_set_bool(data, PREF_PREF_CLOSES, config->close_closes, &err);
	
	gconf_client_set_bool(data, PREF_PREF_NOTIF, config->notification, &err);
	gconf_client_set_int(data, PREF_PREF_NOTIF_TIME, config->notification_time, &err);
	
	gconf_client_set_bool(data, PREF_PREF_PLAY, config->athan_enabled, &err);
	gconf_client_set_string(data, PREF_ATHAN_NORMAL, config->athan_normal, &err);
	gconf_client_set_string(data, PREF_ATHAN_SUBH, config->athan_subh, &err);

	gconf_client_clear_cache(data);

#else
	g_key_file_set_string(data, "city", "name", config->city);
	g_key_file_set_double(data, "city", "latitude", config->latitude);
	g_key_file_set_double(data, "city", "longitude", config->longitude);
	
	g_key_file_set_double(data, "prefs", "correction", config->correction);
	g_key_file_set_integer(data, "prefs", "method", config->method);
	
	g_key_file_set_boolean(data, "prefs", "starthidden", config->start_hidden);
	g_key_file_set_boolean(data, "prefs", "closes", config->close_closes);
	
	g_key_file_set_boolean(data, "prefs", "notif", config->notification);
	g_key_file_set_integer(data, "prefs", "notiftime", config->notification_time);
	
	g_key_file_set_boolean(data, "prefs", "play", config->athan_enabled);
	g_key_file_set_string(data, "athan", "normal", config->athan_normal);
	g_key_file_set_string(data, "athan", "subh", config->athan_subh);
#endif
}
