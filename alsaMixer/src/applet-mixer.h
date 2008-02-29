
#ifndef __APPLET_MIXER__
#define  __APPLET_MIXER__


#include <gtk/gtk.h>


void mixer_init (gchar *cCardID);

void mixer_stop (void);

gchar *mixer_get_elements_list (void);

void mixer_fill_properties (void);

int mixer_get_mean_volume (void);


void mixer_set_volume (int iVolume);

void mixer_switch_mute (void);


GtkWidget *mixer_build_widget (void);

void mixer_show_hide_dialog (void);

gboolean mixer_check_events (gpointer data);

#endif
