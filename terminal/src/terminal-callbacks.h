
#ifndef __TERMINAL_CALLBACKS__
#define  __TERMINAL_CALLBACKS__


#include <cairo-dock.h>



void on_terminal_drag_data_received (GtkWidget *pWidget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data);

gboolean on_terminal_button_press_dialog (GtkWidget* pWidget, GdkEventButton* pButton, gpointer data);

#endif
