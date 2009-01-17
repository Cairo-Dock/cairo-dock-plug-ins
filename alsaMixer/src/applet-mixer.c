/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <alsa/asoundlib.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-mixer.h"

CD_APPLET_INCLUDE_MY_VARS


static int	 mixer_level = 0;
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
	myData.mixer_device_name= g_strdup (snd_ctl_card_info_get_mixername(hw_info));
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
	}
}



gchar *mixer_get_elements_list (void)
{
	snd_mixer_elem_t *elem;
	if (myData.mixer_handle == NULL)
		return NULL;
	cd_message ("");
	
	GString *sMixerElements = g_string_new ("");
	for (elem = snd_mixer_first_elem(myData.mixer_handle); elem; elem = snd_mixer_elem_next(elem))
	{
		if (snd_mixer_selem_is_active (elem) && snd_mixer_selem_has_playback_volume (elem))
			g_string_append_printf (sMixerElements, "%s;", snd_mixer_selem_get_name (elem));
	}
	if (sMixerElements->len > 0)
		sMixerElements->str[sMixerElements->len-1] = '\0';
	
	gchar *cList = sMixerElements->str;
	g_string_free (sMixerElements, FALSE);
	return cList;
}



static snd_mixer_elem_t *_mixer_get_element_by_name (gchar *cName)
{
	if (myData.mixer_handle == NULL)
		return NULL;
	g_return_val_if_fail (cName != NULL, NULL);
	
	snd_mixer_elem_t *elem;
	for (elem = snd_mixer_first_elem(myData.mixer_handle); elem; elem = snd_mixer_elem_next(elem))
	{
		if (strcmp (cName, snd_mixer_selem_get_name (elem)) == 0)
			return elem;
	}
	myData.cErrorMessage = g_strdup_printf (D_("I couldn't find any element '%s'"), cName);
	return NULL;
}

void mixer_get_controlled_element (void)
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
		
	if (bHasLeft)
		snd_mixer_selem_get_playback_volume (myData.pControledElement, SND_MIXER_SCHN_FRONT_LEFT, &iVolumeLeft);
	if (bHasRight)
		snd_mixer_selem_get_playback_volume (myData.pControledElement, SND_MIXER_SCHN_FRONT_RIGHT, &iVolumeRight);
	cd_debug ("volume : %d;%d", iVolumeLeft, iVolumeRight);
	
	g_return_val_if_fail (bHasLeft || bHasRight, 0);
	
	int iMeanVolume = (iVolumeLeft + iVolumeRight) / (bHasLeft + bHasRight);
	
	return ((int) 100. * (iMeanVolume - myData.iVolumeMin) / (myData.iVolumeMax - myData.iVolumeMin));
}

void mixer_set_volume (int iNewVolume)
{
	g_return_if_fail (myData.pControledElement != NULL);
	int iVolume = myData.iVolumeMin + (myData.iVolumeMax - myData.iVolumeMin) * iNewVolume / 100;
	snd_mixer_selem_set_playback_volume_all (myData.pControledElement, iVolume);
	if (myData.pControledElement2 != NULL)
		snd_mixer_selem_set_playback_volume_all (myData.pControledElement2, iVolume);
	myData.iCurrentVolume = iNewVolume;
	mixer_element_update_with_event (myData.pControledElement, 0);  // on ne recoit pas d'evenements pour nos actions.
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

void mixer_switch_mute (void)
{
	g_return_if_fail (myData.pControledElement != NULL);
	gboolean bIsMute = mixer_is_mute ();
	snd_mixer_selem_set_playback_switch_all (myData.pControledElement, bIsMute);
	if (myData.pControledElement2 != NULL)
		snd_mixer_selem_set_playback_switch_all (myData.pControledElement2, bIsMute);
	myData.bIsMute = ! bIsMute;
	mixer_element_update_with_event (myData.pControledElement, 0);  // on ne recoit pas d'evenements pour nos actions.
}



static void on_change_volume (GtkRange *range, gpointer data)
{
	int iNewVolume = (int) gtk_range_get_value (GTK_RANGE (range));
	cd_debug ("%s (%d)", __func__, iNewVolume);
	mixer_set_volume (iNewVolume);
}
GtkWidget *mixer_build_widget (gboolean bHorizontal)
{
	g_return_val_if_fail (myData.pControledElement != NULL, NULL);
	GtkWidget *pScale = (bHorizontal ? gtk_hscale_new_with_range (0., 100., .5*myConfig.iScrollVariation) : gtk_vscale_new_with_range (0., 100., .5*myConfig.iScrollVariation));
	if (! bHorizontal)
		gtk_range_set_inverted (GTK_RANGE (pScale), TRUE);  // de bas en haut.
	
	myData.iCurrentVolume = mixer_get_mean_volume ();
	gtk_range_set_value (GTK_RANGE (pScale), myData.iCurrentVolume);
	
	g_signal_connect (G_OBJECT (pScale),
		"value-changed",
		G_CALLBACK (on_change_volume),
		NULL);
	
	return pScale;
}


void mixer_set_volume_with_no_callback (GtkWidget *pScale, int iVolume)
{
	g_signal_handlers_block_matched (GTK_WIDGET(pScale),
		G_SIGNAL_MATCH_FUNC,
		0, 0, NULL, (void*)on_change_volume, NULL);
	gtk_range_set_value (GTK_RANGE (pScale), (double) iVolume);
	g_signal_handlers_unblock_matched (GTK_WIDGET(pScale),
		G_SIGNAL_MATCH_FUNC,
		0, 0, NULL, (void*)on_change_volume, NULL);
}


static gboolean on_button_press_dialog (GtkWidget *widget,
	GdkEventButton *pButton,
	CairoDialog *pDialog)
{
	cairo_dock_dialog_unreference (pDialog);
	myData.pDialog = NULL;
	return FALSE;
}

void mixer_show_hide_dialog (void)
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
			cMessage = D_("Set up volume :");
			pScale = mixer_build_widget (TRUE);
		}
		
		CairoDialogAttribute attr;
		memset (&attr, 0, sizeof (CairoDialogAttribute));
		attr.cText = cMessage;
		attr.pInteractiveWidget = pScale;
		CairoDialog *pDialog = cairo_dock_build_dialog (&attr, myIcon, myContainer);
		/*myData.pDialog = cairo_dock_show_dialog_full (cMessage,
			myIcon,
			myContainer,
			0,
			NULL,
			GTK_BUTTONS_NONE,
			pScale,
			NULL,
			NULL,
			NULL);*/
		g_signal_connect (G_OBJECT (myData.pDialog->pWidget),
			"button-press-event",
			G_CALLBACK (on_button_press_dialog),
			myData.pDialog);
	}
	else
	{
		cairo_dock_dialog_unreference (myData.pDialog);
		myData.pDialog = NULL;
	}
}

gboolean mixer_check_events (gpointer data)
{
	snd_mixer_handle_events (myData.mixer_handle);  // ne renvoie pas d'evenements pour nos actions !
	return TRUE;
}
