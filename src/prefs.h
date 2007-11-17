/* prefs.h
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

#if USE_GCONF
#include <gconf/gconf-client.h>
#define MINBAR_PREFDIR 		"/apps/minbar"
#define PREF_CITY_LAT 		MINBAR_PREFDIR "/city/lat"
#define PREF_CITY_LON  		MINBAR_PREFDIR "/city/lon"
#define PREF_CITY_NAME 		MINBAR_PREFDIR "/city/name"
#define PREF_CITY_HEIGHT	MINBAR_PREFDIR "/city/height"
#define PREF_CITY_CORRECTION  	MINBAR_PREFDIR "/prefs/correctiond"
#define PREF_PREF_PLAY  	MINBAR_PREFDIR "/prefs/play"
#define PREF_PREF_NOTIF  	MINBAR_PREFDIR "/prefs/notif"
#define PREF_PREF_NOTIF_TIME 	MINBAR_PREFDIR "/prefs/notiftime"
#define PREF_PREF_METHOD 	MINBAR_PREFDIR "/prefs/method"
#define PREF_PREF_HIDDEN 	MINBAR_PREFDIR "/prefs/starthidden"
#define PREF_PREF_CLOSES 	MINBAR_PREFDIR "/prefs/closes"
#define PREF_ATHAN_NORMAL 	MINBAR_PREFDIR "/athan/normal"
#define PREF_ATHAN_SUBH 	MINBAR_PREFDIR "/athan/subh"

#else
#include <glib.h>

#endif
