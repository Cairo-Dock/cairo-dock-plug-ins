
#ifndef __APPLET_CLIPBOARD__
#define  __APPLET_CLIPBOARD__


#include <cairo-dock.h>
#include "applet-struct.h"


void _on_text_received (GtkClipboard *pClipBoard, const gchar *text, gpointer user_data); // temporairement declaree ici.

void cd_clipper_selection_owner_changed (GtkClipboard *pClipBoard, GdkEvent *event, gpointer user_data);


GList *cd_clipper_load_actions (const gchar *cConfFilePath);

void cd_clipper_free_command (CDClipperCommand *pCommand);

void cd_clipper_free_action (CDClipperAction *pAction);


GtkWidget *cd_clipper_build_action_menu (CDClipperAction *pAction);

GtkWidget *cd_clipper_build_items_menu (void);

GtkWidget *cd_clipper_build_persistent_items_menu (void);

void cd_clipper_show_menu (GtkWidget *pMenu, gint iButton);

#endif
