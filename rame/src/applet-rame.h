#ifndef __APPLET_RAME__
#define  __APPLET_RAME__

#include <cairo-dock.h>
/*gboolean inDebug;
gboolean cd_rame_timer(gpointer data);
gboolean cd_rame_getRate(void);
void cd_rame_get_data (void);
void cd_rame_launch_analyse(void);*/
void cd_rame_formatRate(unsigned long long rate, gchar* debit);


void cd_rame_read_data (void);
void cd_rame_update_from_data (void);


#endif
