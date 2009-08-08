
#ifndef __APPLET_COMMAND_FINDER__
#define  __APPLET_COMMAND_FINDER__


#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_do_check_locate_is_available (void);

void cd_do_find_matching_files (void);


void cd_do_activate_filter_option (int iNumOption);


void cd_do_show_current_sub_listing (void);

void cd_do_show_previous_listing (void);


void cd_do_filter_current_listing (void);


#endif
