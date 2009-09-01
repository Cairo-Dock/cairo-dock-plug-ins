/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


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
