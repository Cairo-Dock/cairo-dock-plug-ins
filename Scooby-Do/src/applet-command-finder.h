
#ifndef __APPLET_COMMAND_FINDER__
#define  __APPLET_COMMAND_FINDER__


#include <cairo-dock.h>
#include "applet-struct.h"


gboolean cd_do_check_locate_is_available (void);


void cd_do_update_completion (const char *text);


void cd_do_find_matching_files (void);


void cd_do_reset_applications_list (void);

void cd_do_find_matching_applications (void);


void cd_do_show_filter_dialog (void);

void cd_do_hide_filter_dialog (void);


#endif
