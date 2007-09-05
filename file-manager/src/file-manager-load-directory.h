
#ifndef __FILE_MANAGER_LOAD_DIRECTORY__
#define  __FILE_MANAGER_LOAD_DIRECTORY__


#include <cairo-dock.h>


void file_manager_create_dock_from_directory (Icon *pIcon);


void file_monitor_action_on_event (int iEventType, const gchar *cURI, Icon *pIcon);


#endif
