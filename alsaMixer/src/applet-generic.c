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

int cd_get_capture_volume (void)
{
	if (myData.ctl.get_capture_volume)
		return myData.ctl.get_capture_volume ();
	return 0;
}

void cd_set_capture_volume (int iVolume)
{
	if (myData.ctl.set_capture_volume)
		myData.ctl.set_capture_volume (iVolume);
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
static void on_change_capture_volume (GtkRange *range, gpointer data)
{
	CD_APPLET_ENTER;
	int iNewVolume = (int) gtk_range_get_value (GTK_RANGE (range));
	cd_debug ("%s (%d)", __func__, iNewVolume);
	cd_set_capture_volume (iNewVolume);
	CD_APPLET_LEAVE();
}
GtkWidget *mixer_build_widget (gboolean bHorizontal)
{
	g_return_val_if_fail (myData.playback.pControledElement != NULL, NULL);
	// build the playback volume scale
	GtkWidget *pScale = gtk_scale_new_with_range (bHorizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0., 100., .5*myConfig.iScrollVariation);
	if (! bHorizontal)
		gtk_range_set_inverted (GTK_RANGE (pScale), TRUE);  // de bas en haut.
	
	myData.playback.iCurrentVolume = cd_get_volume ();
	gtk_range_set_value (GTK_RANGE (pScale), myData.playback.iCurrentVolume);
	gtk_range_set_increments(GTK_RANGE (pScale), 5, 5);
	
	g_signal_connect (G_OBJECT (pScale),
		"value-changed",
		G_CALLBACK (on_change_volume),
		NULL);
	
	myData.pPlaybackScale = pScale;
	
	// build the capture volume scale
	if (myData.capture.pControledElement != NULL)
	{
		GtkWidget *pScale2 = gtk_scale_new_with_range (bHorizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0., 100., .5*myConfig.iScrollVariation);
		if (! bHorizontal)
			gtk_range_set_inverted (GTK_RANGE (pScale2), TRUE);  // de bas en haut.
		
		myData.capture.iCurrentVolume = cd_get_capture_volume ();
		gtk_range_set_value (GTK_RANGE (pScale2), myData.capture.iCurrentVolume);
		gtk_range_set_increments(GTK_RANGE (pScale2), 5, 5);
		
		g_signal_connect (G_OBJECT (pScale2),
			"value-changed",
			G_CALLBACK (on_change_capture_volume),
			NULL);
		
		myData.pCaptureScale = pScale2;
			
		// add both scales with some icons into a box
		GtkWidget *box = gtk_box_new (bHorizontal ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL, 0);
		
		GtkWidget *hbox = gtk_box_new (bHorizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0);
		GtkWidget *icon = gtk_image_new_from_icon_name ("audio-speakers", GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), pScale, TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);
		
		hbox = gtk_box_new (bHorizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0);
		icon = gtk_image_new_from_icon_name ("audio-input-microphone", GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), pScale2, TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);
		return box;
	}
	
	return pScale;
}


void cd_mixer_set_volume_with_no_callback (GtkWidget *pScale, int iVolume)
{
	g_signal_handlers_block_matched (GTK_WIDGET(pScale),
		G_SIGNAL_MATCH_FUNC,
		0, 0, NULL, (void*)on_change_volume, NULL);
	g_signal_handlers_block_matched (GTK_WIDGET(pScale),
		G_SIGNAL_MATCH_FUNC,
		0, 0, NULL, (void*)on_change_capture_volume, NULL);
	gtk_range_set_value (GTK_RANGE (pScale), (double) iVolume);
	g_signal_handlers_unblock_matched (GTK_WIDGET(pScale),
		G_SIGNAL_MATCH_FUNC,
		0, 0, NULL, (void*)on_change_volume, NULL);
	g_signal_handlers_unblock_matched (GTK_WIDGET(pScale),
		G_SIGNAL_MATCH_FUNC,
		0, 0, NULL, (void*)on_change_capture_volume, NULL);
}
