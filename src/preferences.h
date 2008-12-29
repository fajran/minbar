#ifndef PREFERENCES_H_
#define PREFERENCES_H_

#include <gtk/gtk.h>

extern MinbarConfig *config;

void preferences_show(void);
void preferences_set_notification(gboolean);

#endif
