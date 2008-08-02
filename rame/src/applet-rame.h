#ifndef __APPLET_RAME__
#define  __APPLET_RAME__

#include <cairo-dock.h>


void cd_rame_formatRate(unsigned long long rate, gchar* debit);


void cd_rame_read_data (CairoDockModuleInstance *myApplet);
gboolean cd_rame_update_from_data (CairoDockModuleInstance *myApplet);


void cd_rame_free_process (CDProcess *pProcess);

void cd_rame_get_process_memory (void);

void cd_rame_clean_all_processes (void);


#endif
