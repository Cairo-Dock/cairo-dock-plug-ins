/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* based on indicator-messages.c written by :
*  Ted Gould <ted@canonical.com>
*  Cody Russell <cody.russell@canonical.com>
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

#include <stdlib.h>

#include "applet-struct.h"
#include "applet-backend-alsamixer.h"
#ifdef INDICATOR_SOUNDMENU_WITH_IND3
#include "applet-backend-sound-menu.h"
#elif defined SOUND_SERVICE_SUPPORT // OLD
#include "applet-backend-sound-menu-old.h"
#endif
#include "applet-generic.h"


int cd_get_volume (void)
{
	if (myData.ctl.get_volume)
		return myData.ctl.get_volume ();
	return 0;
}

void cd_set_volume (int iVolume)
{
	if (myData.ctl.set_volume)
		myData.ctl.set_volume (iVolume);
}

void cd_toggle_mute (void)
{
	if (myData.ctl.toggle_mute)
		myData.ctl.toggle_mute ();
}

void cd_show_hide (void)
{
	if (myData.ctl.show_hide)
		myData.ctl.show_hide ();
}

void cd_stop (void)
{
	if (myData.ctl.stop)
		myData.ctl.stop ();
}

void cd_reload (void)
{
	if (myData.ctl.reload)
		myData.ctl.reload ();
}

void cd_start (void)
{
	#if defined INDICATOR_SOUNDMENU_WITH_IND3 || defined SOUND_SERVICE_SUPPORT
	cd_mixer_connect_to_sound_service ();  // connect to the sound service, it will fall back to alsa if it's not available.
	#else
	cd_mixer_init_alsa ();
	#endif

}


/// SCALE ///

static void on_change_volume (GtkRange *range, gpointer data)
{
	CD_APPLET_ENTER;
	int iNewVolume = (int) gtk_range_get_value (GTK_RANGE (range));
	cd_debug ("%s (%d)", __func__, iNewVolume);
	cd_set_volume (iNewVolume);
	CD_APPLET_LEAVE();
}
GtkWidget *mixer_build_widget (gboolean bHorizontal)
{
	g_return_val_if_fail (myData.pControledElement != NULL, NULL);
	GtkWidget *pScale = gtk_scale_new_with_range (bHorizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0., 100., .5*myConfig.iScrollVariation);
	if (! bHorizontal)
		gtk_range_set_inverted (GTK_RANGE (pScale), TRUE);  // de bas en haut.
	
	myData.iCurrentVolume = cd_get_volume ();
	gtk_range_set_value (GTK_RANGE (pScale), myData.iCurrentVolume);
	
	g_signal_connect (G_OBJECT (pScale),
		"value-changed",
		G_CALLBACK (on_change_volume),
		NULL);
	
	gldi_dialog_set_widget_text_color (pScale);
	return pScale;
}


void cd_mixer_set_volume_with_no_callback (GtkWidget *pScale, int iVolume)
{
	g_signal_handlers_block_matched (GTK_WIDGET(pScale),
		G_SIGNAL_MATCH_FUNC,
		0, 0, NULL, (void*)on_change_volume, NULL);
	gtk_range_set_value (GTK_RANGE (pScale), (double) iVolume);
	g_signal_handlers_unblock_matched (GTK_WIDGET(pScale),
		G_SIGNAL_MATCH_FUNC,
		0, 0, NULL, (void*)on_change_volume, NULL);
}
