
#ifndef __APPLET_MIXER__
#define  __APPLET_MIXER__


#include <gtk/gtk.h>


void mixer_init (gchar *cCardID);

void mixer_stop (void);

GList *mixer_get_elements_list (void);

void mixer_get_controlled_element (void);


int mixer_get_mean_volume (void);


void mixer_set_volume (int iNewVolume);

gboolean mixer_is_mute (void);

void mixer_switch_mute (void);


GtkWidget *mixer_build_widget (gboolean bHorizontal);

void mixer_set_volume_with_no_callback (GtkWidget *pScale, int iVolume);

void mixer_show_hide_dialog (void);

gboolean mixer_check_events (gpointer data);

#endif
