
#ifndef __FILE_MANAGER_ADD_DESKTOP_FILE__
#define  __FILE_MANAGER_ADD_DESKTOP_FILE__


#include <cairo-dock.h>


gchar * file_manager_add_desktop_file_from_uri (gchar *cURI, gchar *cDockName, double fOrder, CairoDock *pDock, GError **erreur);


#endif
