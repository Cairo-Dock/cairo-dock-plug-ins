
#ifndef __APPLET_CLIPBOARD__
#define  __APPLET_CLIPBOARD__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_clipper_selection_owner_changed (GtkClipboard *pClipBoard, GdkEvent *event, gpointer user_data);


#endif
