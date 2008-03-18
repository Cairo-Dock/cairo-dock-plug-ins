
#ifndef __APPLET_BOOKMARKS__
#define  __APPLET_BOOKMARKS__

#include <cairo-dock.h>


void cd_shortcuts_on_change_bookmarks (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data);


void cd_shortcuts_remove_one_bookmark (const gchar *cURI);

void cd_shortcuts_add_one_bookmark (const gchar *cURI);


GList *cd_shortcuts_list_bookmarks (gchar *cBookmarkFilePath);


#endif
