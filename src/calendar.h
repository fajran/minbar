#ifndef CALENDAR_H_
#define CALENDAR_H_

#include <gtk/gtk.h>
#include <itl/prayer.h>
#include "config.h"

extern MinbarConfig *config;
extern gchar *time_names[6];
extern Location *loc;
extern Method *calcMethod;
extern Date *prayerDate;


void calendar_show(void);
void calendar_update(void);

#endif
