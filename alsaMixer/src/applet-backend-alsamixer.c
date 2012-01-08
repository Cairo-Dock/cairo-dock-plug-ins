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

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <glib/gi18n.h>
#include <alsa/asoundlib.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-generic.h"
#include "applet-backend-alsamixer.h"

static int mixer_level = 0;
static struct snd_mixer_selem_regopt mixer_options;


static int
mixer_event (snd_mixer_t *mixer, unsigned int mask, snd_mixer_elem_t *elem)
{
	cd_debug ("");
	return 0;
}

void mixer_init (gchar *cCardID)  // this function is taken from AlsaMixer.
{
	snd_ctl_card_info_t *hw_info = NULL;  // ne pas liberer.
	snd_ctl_t *ctl_handle = NULL;
	int err;
	snd_ctl_card_info_alloca (&hw_info);
	
	if ((err = snd_ctl_open (&ctl_handle, cCardID, 0)) < 0)
	{
		myData.cErrorMessage = g_strdup_printf (D_("I couldn't open card '%s'"), cCardID);
		return ;
	}
	if ((err = snd_ctl_card_info (ctl_handle, hw_info)) < 0)
	{
		myData.cErrorMessage = g_strdup_printf (D_("Card '%s' opened but I couldn't get any info"), cCardID);
		return ;
	}
	snd_ctl_close (ctl_handle);
	
	// open mixer device
	if ((err = snd_mixer_open (&myData.mixer_handle, 0)) < 0)
	{
		myData.cErrorMessage = g_strdup (D_("I couldn't open the mixer"));
		return ;
	}
	if (mixer_level == 0 && (err = snd_mixer_attach (myData.mixer_handle, cCardID)) < 0)
	{
		snd_mixer_free (myData.mixer_handle);
		myData.mixer_handle = NULL;
		myData.cErrorMessage = g_strdup (D_("I couldn't attach the mixer to the card"));
		return ;
	}
	if ((err = snd_mixer_selem_register (myData.mixer_handle, mixer_level > 0 ? &mixer_options : NULL, NULL)) < 0)
	{
		snd_mixer_free (myData.mixer_handle);
		myData.mixer_handle = NULL;
		myData.cErrorMessage = g_strdup (D_("I couldn't register options"));
		return ;
	}
	///snd_mixer_set_callback (myData.mixer_handle, mixer_event);
	if ((err = snd_mixer_load (myData.mixer_handle)) < 0)
	{
		snd_mixer_free (myData.mixer_handle);
		myData.mixer_handle = NULL;
		myData.cErrorMessage = g_strdup (D_("I couldn't load the mixer"));
		return ;
	}
	
	myData.mixer_card_name = g_strdup (snd_ctl_card_info_get_name(hw_info));
	myData.mixer_device_name = g_strdup (snd_ctl_card_info_get_mixername(hw_info));
	cd_debug ("myData.mixer_card_name : %s ; myData.mixer_device_name : %s", myData.mixer_card_name, myData.mixer_device_name);
}

void mixer_stop (void)
{
	if (myData.mixer_handle != NULL)
	{
		snd_mixer_detach (myData.mixer_handle, myConfig.card_id);
		snd_mixer_close (myData.mixer_handle);
		myData.mixer_handle = NULL;
		myData.pControledElement = NULL;
		myData.pControledElement2 = NULL;
		
		g_free (myData.cErrorMessage);
		myData.cErrorMessage = NULL;
		g_free (myData.mixer_card_name);
		myData.mixer_card_name = NULL;
		g_free (myData.mixer_device_name);
		myData.mixer_device_name= NULL;
	}
}


GList *mixer_get_elements_list (void)
{
	snd_mixer_elem_t *elem;
	if (myData.mixer_handle == NULL)
		return NULL;
	cd_message ("");
	
	GList *pList = NULL;
	for (elem = snd_mixer_first_elem(myData.mixer_handle); elem; elem = snd_mixer_elem_next(elem))
	{
		if (snd_mixer_selem_is_active (elem) && snd_mixer_selem_has_playback_volume (elem))
			pList = g_list_prepend (pList, (gpointer)snd_mixer_selem_get_name (elem));  // la liste ne contiendra que des const, on ne supprimera pas ses elements lors du g_list_free.
	}
	return pList;
}


static snd_mixer_elem_t *_mixer_get_element_by_name (const gchar *cName)
{
	if (myData.mixer_handle == NULL)
		return NULL;
	
	if (cName != NULL)
	{
		int i = 0;
		snd_mixer_elem_t *elem;
		for (elem = snd_mixer_first_elem(myData.mixer_handle); elem; elem = snd_mixer_elem_next(elem))
		{
			if (strcmp (cName, snd_mixer_selem_get_name (elem)) == 0)
				return elem;
		}
	}
	
	cd_debug ("no channel matches '%s', we take the first available channel by default", cName);
	return snd_mixer_first_elem(myData.mixer_handle);
	/**myData.cErrorMessage = g_strdup_printf (D_("I couldn't find any audio channel named '%s'\nYou should try to open the configuration panel of the applet,\n and select the proper audio channel you want to control."), cName);
	return NULL;*/
}


static int mixer_element_update_with_event (snd_mixer_elem_t *elem, unsigned int mask)
{
	CD_APPLET_ENTER;
	cd_debug ("%s (%d)", __func__, mask);
	
	if (mask != 0)
	{
		myData.iCurrentVolume = mixer_get_mean_volume ();
		myData.bIsMute = mixer_is_mute ();
		cd_debug (" iCurrentVolume <- %d bIsMute <- %d", myData.iCurrentVolume, myData.bIsMute);
	}
	
	cd_update_icon ();
	
	CD_APPLET_LEAVE(0);
}

static void mixer_get_controlled_element (void)
{
	myData.pControledElement = _mixer_get_element_by_name (myConfig.cMixerElementName);
	if (myData.pControledElement != NULL)
	{
		myData.bHasMuteSwitch = snd_mixer_selem_has_playback_switch (myData.pControledElement);
		
		snd_mixer_selem_get_playback_volume_range (myData.pControledElement, &myData.iVolumeMin, &myData.iVolumeMax);
		cd_debug ("volume range : %d - %d", myData.iVolumeMin, myData.iVolumeMax);
		
		snd_mixer_elem_set_callback (myData.pControledElement, mixer_element_update_with_event);
	}
	if (myConfig.cMixerElementName2 != NULL)
	{
		myData.pControledElement2 = _mixer_get_element_by_name (myConfig.cMixerElementName2);
	}
}

int mixer_get_mean_volume (void)
{
	g_return_val_if_fail (myData.pControledElement != NULL, 0);
	long iVolumeLeft=0, iVolumeRight=0;
	gboolean bHasLeft = snd_mixer_selem_has_playback_channel (myData.pControledElement, SND_MIXER_SCHN_FRONT_LEFT);
	gboolean bHasRight = snd_mixer_selem_has_playback_channel (myData.pControledElement, SND_MIXER_SCHN_FRONT_RIGHT);
	g_return_val_if_fail (bHasLeft || bHasRight, 0);
	
	if (bHasLeft)
		snd_mixer_selem_get_playback_volume (myData.pControledElement, SND_MIXER_SCHN_FRONT_LEFT, &iVolumeLeft);
	if (bHasRight)
		snd_mixer_selem_get_playback_volume (myData.pControledElement, SND_MIXER_SCHN_FRONT_RIGHT, &iVolumeRight);
	cd_debug ("volume : %d;%d", iVolumeLeft, iVolumeRight);
	
	int iMeanVolume = (iVolumeLeft + iVolumeRight) / (bHasLeft + bHasRight);
	
	cd_debug ("myData.iVolumeMin : %d ; myData.iVolumeMax : %d ; iMeanVolume : %d", myData.iVolumeMin, myData.iVolumeMax, iMeanVolume);
	return (100. * (iMeanVolume - myData.iVolumeMin) / (myData.iVolumeMax - myData.iVolumeMin));
}

static void mixer_set_volume (int iNewVolume)
{
	g_return_if_fail (myData.pControledElement != NULL);
	cd_debug ("%s (%d)", __func__, iNewVolume);
	int iVolume = ceil (myData.iVolumeMin + (myData.iVolumeMax - myData.iVolumeMin) * iNewVolume / 100.);
	snd_mixer_selem_set_playback_volume_all (myData.pControledElement, iVolume);
	if (myData.pControledElement2 != NULL)
		snd_mixer_selem_set_playback_volume_all (myData.pControledElement2, iVolume);
	myData.iCurrentVolume = iNewVolume;
	cd_update_icon ();  // on ne recoit pas d'evenements pour nos actions.
}

gboolean mixer_is_mute (void)
{
	cd_debug ("");
	g_return_val_if_fail (myData.pControledElement != NULL, FALSE);
	if (snd_mixer_selem_has_playback_switch (myData.pControledElement))
	{
		int iSwitchLeft, iSwitchRight;
		snd_mixer_selem_get_playback_switch (myData.pControledElement, SND_MIXER_SCHN_FRONT_LEFT, &iSwitchLeft);
		snd_mixer_selem_get_playback_switch (myData.pControledElement, SND_MIXER_SCHN_FRONT_RIGHT, &iSwitchRight);
		cd_debug ("%d;%d", iSwitchLeft, iSwitchRight);
		return (iSwitchLeft == 0 && iSwitchRight == 0);
	}
	else
		return FALSE;
}

static void mixer_switch_mute (void)
{
	g_return_if_fail (myData.pControledElement != NULL);
	gboolean bIsMute = mixer_is_mute ();
	snd_mixer_selem_set_playback_switch_all (myData.pControledElement, bIsMute);
	if (myData.pControledElement2 != NULL)
		snd_mixer_selem_set_playback_switch_all (myData.pControledElement2, bIsMute);
	myData.bIsMute = ! bIsMute;
	cd_update_icon ();  // on ne recoit pas d'evenements pour nos actions.
}



static void _on_dialog_destroyed (CairoDockModuleInstance *myApplet)
{
	myData.pDialog = NULL;
}
static void mixer_show_hide_dialog (void)
{
	if (myDesklet)
		return ;
	if (myData.pDialog == NULL)
	{
		const gchar *cMessage;
		GtkWidget *pScale = NULL;
		if (myData.cErrorMessage != NULL)
			cMessage = myData.cErrorMessage;
		else
		{
			cMessage = D_("Set up volume:");
			pScale = mixer_build_widget (TRUE);
		}
		
		CairoDialogAttribute attr;
		memset (&attr, 0, sizeof (CairoDialogAttribute));
		attr.cText = cMessage;
		attr.cImageFilePath = MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE;
		attr.pInteractiveWidget = pScale;
		attr.pUserData = myApplet;
		attr.pFreeDataFunc = (GFreeFunc)_on_dialog_destroyed;
		
		myData.pDialog = cairo_dock_build_dialog (&attr, myIcon, myContainer);
	}
	else
	{
		cairo_dock_dialog_unreference (myData.pDialog);
		myData.pDialog = NULL;
	}
}

gboolean mixer_check_events (gpointer data)
{
	CD_APPLET_ENTER;
	snd_mixer_handle_events (myData.mixer_handle);  // ne renvoie pas d'evenements pour nos actions !
	CD_APPLET_LEAVE(TRUE);
}


static void cd_mixer_stop_alsa (void)
{
	if (myData.mixer_handle != NULL)
	{
		mixer_stop ();
		
		g_free (myData.cErrorMessage);
		myData.cErrorMessage = NULL;
		g_free (myData.mixer_card_name);
		myData.mixer_card_name = NULL;
		g_free (myData.mixer_device_name);
		myData.mixer_device_name= NULL;
		
		if (myData.iSidCheckVolume != 0)
		{
			g_source_remove (myData.iSidCheckVolume);
			myData.iSidCheckVolume = 0;
		}
	}
}

static void cd_mixer_reload_alsa (void)
{
	cd_mixer_stop_alsa ();
	
	mixer_init (myConfig.card_id);
	mixer_get_controlled_element ();
	
	if (myData.pControledElement == NULL)
	{
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cBrokenIcon, "broken.svg");
	}
	else
	{
		mixer_element_update_with_event (myData.pControledElement, 1);  // 1 => get the current state (card may have changed).
		
		myData.iSidCheckVolume = g_timeout_add (1000, (GSourceFunc) mixer_check_events, (gpointer) NULL);
	}
}

void cd_mixer_init_alsa (void)
{
	// connect to the sound card
	mixer_init (myConfig.card_id);
	
	// get the mixer element
	mixer_get_controlled_element ();
	
	// update the icon
	if (myData.pControledElement == NULL)  // no luck
	{
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cBrokenIcon, "broken.svg");
	}
	else  // mixer aquired
	{
		// set the interface
		myData.ctl.get_volume = mixer_get_mean_volume;
		myData.ctl.set_volume = mixer_set_volume;
		myData.ctl.toggle_mute = mixer_switch_mute;
		myData.ctl.show_hide = mixer_show_hide_dialog;
		myData.ctl.stop = cd_mixer_stop_alsa;
		myData.ctl.reload = cd_mixer_reload_alsa;
		
		// build the scale now if we're in a desklet
		if (myDesklet)
		{
			GtkWidget *box = _gtk_hbox_new (0);
			myData.pScale = mixer_build_widget (FALSE);
			gtk_box_pack_end (GTK_BOX (box), myData.pScale, FALSE, FALSE, 0);
			gtk_container_add (GTK_CONTAINER (myDesklet->container.pWidget), box);
			gtk_widget_show_all (box);
			
			if (myConfig.bHideScaleOnLeave && ! myDesklet->container.bInside)
				gtk_widget_hide (myData.pScale);
		}
		else if (myIcon->cName == NULL)  // in dock, set the label
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (myData.mixer_card_name);
		}
		
		// trigger the callback to update the icon
		mixer_element_update_with_event (myData.pControledElement, 1);  // 1 => get the current state.
		myData.iSidCheckVolume = g_timeout_add (1000, (GSourceFunc) mixer_check_events, (gpointer) NULL);
	}
}
