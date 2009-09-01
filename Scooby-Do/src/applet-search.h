
#ifndef __APPLET_SEARCH__
#define  __APPLET_SEARCH__


#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_do_fill_default_entry (CDEntry *pEntry);


void cd_do_launch_backend (CDBackend *pBackend);
void cd_do_launch_all_backends (void);

void cd_do_stop_backend (CDBackend *pBackend);
void cd_do_stop_all_backends (void);


void cd_do_append_entries_to_listing (GList *pEntries, gint iNbEntries);

void cd_do_remove_entries_from_listing (CDBackend *pBackend);

int cd_do_filter_entries (GList *pEntries, gint iNbEntries);



void cd_do_activate_filter_option (int iNumOption);


void cd_do_show_current_sub_listing (void);

void cd_do_show_previous_listing (void);


void cd_do_filter_current_listing (void);


#endif
