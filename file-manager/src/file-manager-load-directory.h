
#ifndef __FILE_MANAGER_LOAD_DIRECTORY__
#define  __FILE_MANAGER_LOAD_DIRECTORY__


#include <cairo-dock.h>

#include "file-manager-struct.h"

void file_manager_create_dock_from_directory (Icon *pIcon);

Icon *file_manager_create_icon_from_URI (gchar *cURI, CairoDock *pDock);


void file_monitor_action_on_event (FileManagerEventType iEventType, const gchar *cURI, Icon *pIcon);


void file_manager_reload_directories (gchar *cName, CairoDock *pDock, gpointer data);
void file_manager_unload_directories (gchar *cName, CairoDock *pDock, gboolean *bSomethingUnloaded);


//GList *file_manager_sort_files (GList *pIconList, FileManagerSortType iSortType);


#endif
