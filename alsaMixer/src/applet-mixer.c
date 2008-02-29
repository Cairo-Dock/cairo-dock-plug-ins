/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

Inspiration was taken from the well-known AlsaMixer.

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <alsa/asoundlib.h>

#include "applet-struct.h"
#include "applet-mixer.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

static int	 mixer_level = 0;
static struct snd_mixer_selem_regopt mixer_options;
static int mixer_changed_state=0;

static int
mixer_event (snd_mixer_t *mixer, unsigned int mask, snd_mixer_elem_t *elem)
{
	mixer_changed_state = 1;
	g_print ("%s ()\n", __func__);
	return 0;
}
static int
mixer_element_event (snd_mixer_elem_t *elem, unsigned int mask)
{
	mixer_changed_state = 1;
	g_print ("%s (%d)\n", __func__, mask);
	
	myData.iCurrentVolume = mixer_get_mean_volume ();
	g_print (" iCurrentVolume <- %d\n", myData.iCurrentVolume);
	
	switch (myConfig.iVolumeDisplay)
	{
		case VOLUME_NO_DISPLAY :
		break;
		
		case VOLUME_ON_LABEL :
		{
			gchar *cLabel = g_strdup_printf ("%s : %d%%", myData.mixer_card_name, myData.iCurrentVolume);
			CD_APPLET_SET_NAME_FOR_MY_ICON (cLabel)
			g_free (cLabel);
		}
		break;
		
		case VOLUME_ON_ICON :
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_AND_REDRAW ("%d%%", myData.iCurrentVolume)
		break;
		
		default :
		break;
	}
	
	return 0;
}

void mixer_init (gchar *cCardID)
{
	snd_ctl_card_info_t *hw_info = NULL;  // ne pas liberer.
	snd_ctl_t *ctl_handle = NULL;
	int err;
	snd_ctl_card_info_alloca (&hw_info);
	
	if ((err = snd_ctl_open (&ctl_handle, cCardID, 0)) < 0)
	{
		g_print ("Attention : couldn't open card %s\n", cCardID);
		return ;
	}
	if ((err = snd_ctl_card_info (ctl_handle, hw_info)) < 0)
	{
		g_print ("Attention : couldn't get card info\n");
		return ;
	}
	snd_ctl_close (ctl_handle);
	
	// open mixer device
	if ((err = snd_mixer_open (&myData.mixer_handle, 0)) < 0)
		return ;
	if (mixer_level == 0 && (err = snd_mixer_attach (myData.mixer_handle, cCardID)) < 0)
	{
		snd_mixer_free (myData.mixer_handle);
		myData.mixer_handle = NULL;
		return ;
	}
	if ((err = snd_mixer_selem_register (myData.mixer_handle, mixer_level > 0 ? &mixer_options : NULL, NULL)) < 0)
	{
		snd_mixer_free (myData.mixer_handle);
		myData.mixer_handle = NULL;
		return ;
	}
	snd_mixer_set_callback (myData.mixer_handle, mixer_event);
	if ((err = snd_mixer_load (myData.mixer_handle)) < 0)
	{
		snd_mixer_free (myData.mixer_handle);
		myData.mixer_handle = NULL;
		return ;
	}
	
	myData.mixer_card_name = g_strdup (snd_ctl_card_info_get_name(hw_info));
	myData.mixer_device_name= g_strdup (snd_ctl_card_info_get_mixername(hw_info));
}

void mixer_stop (void)
{
	if (myData.mixer_handle)
	{
		snd_mixer_detach (myData.mixer_handle, myConfig.card_id);
		snd_mixer_close (myData.mixer_handle);
		myData.mixer_handle = NULL;
	}
}



gchar *mixer_get_elements_list (void)
{
	snd_mixer_elem_t *elem;
	if (myData.mixer_handle == NULL)
		return NULL;
	g_print ("%s ()\n", __func__);
	
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

static snd_mixer_elem_t *mixer_get_element_by_name (gchar *cName)
{
	g_return_val_if_fail (cName != NULL && myData.mixer_handle != NULL, NULL);
	
	snd_mixer_elem_t *elem;
	for (elem = snd_mixer_first_elem(myData.mixer_handle); elem; elem = snd_mixer_elem_next(elem))
	{
		if (strcmp (cName, snd_mixer_selem_get_name (elem)) == 0)
			return elem;
	}
	return NULL;
}

void mixer_fill_properties (void)
{
	myData.pControledElement = mixer_get_element_by_name (myConfig.cMixerElementName);
	if (myData.pControledElement != NULL)
	{
		snd_mixer_selem_id_malloc (&myData.pControledID);
		snd_mixer_selem_get_id (myData.pControledElement, myData.pControledID);
		
		myData.bHasMuteSwitch = snd_mixer_selem_has_playback_switch (myData.pControledElement);
		snd_mixer_selem_get_playback_volume_range (myData.pControledElement, &myData.iVolumeMin, &myData.iVolumeMax);
		g_print ("volume range : %d - %d\n", myData.iVolumeMin, myData.iVolumeMax);
		
		snd_mixer_elem_set_callback (myData.pControledElement, mixer_element_event);
	}
}

int mixer_get_mean_volume (void)
{
	long iVolumeLeft=0, iVolumeRight=0;
	gboolean bHasLeft = snd_mixer_selem_has_playback_channel (myData.pControledElement, SND_MIXER_SCHN_FRONT_LEFT);
	gboolean bHasRight = snd_mixer_selem_has_playback_channel (myData.pControledElement, SND_MIXER_SCHN_FRONT_RIGHT);
		
	if (bHasLeft)
		snd_mixer_selem_get_playback_volume (myData.pControledElement, SND_MIXER_SCHN_FRONT_LEFT, &iVolumeLeft);
	if (bHasRight)
		snd_mixer_selem_get_playback_volume (myData.pControledElement, SND_MIXER_SCHN_FRONT_RIGHT, &iVolumeRight);
	g_print ("volume : %d;%d\n", iVolumeLeft, iVolumeRight);
	
	g_return_val_if_fail (bHasLeft || bHasRight, 0);
	
	int iMeanVolume = (iVolumeLeft + iVolumeRight) / (bHasLeft + bHasRight);
	
	return ((int) 100. * (iMeanVolume - myData.iVolumeMin) / (myData.iVolumeMax - myData.iVolumeMin));
}


void mixer_set_volume (int iVolume)
{
	snd_mixer_selem_set_playback_volume_all (myData.pControledElement, iVolume);
}

void mixer_switch_mute (void)
{
	if (snd_mixer_selem_has_playback_switch (myData.pControledElement))
	{
		int iSwitchLeft, iSwitchRight;
		snd_mixer_selem_get_playback_switch (myData.pControledElement, SND_MIXER_SCHN_FRONT_LEFT, &iSwitchLeft);
		snd_mixer_selem_get_playback_switch (myData.pControledElement, SND_MIXER_SCHN_FRONT_RIGHT, &iSwitchRight);
		if (iSwitchLeft == 0 || iSwitchRight == 0)
			snd_mixer_selem_set_playback_switch_all (myData.pControledElement, 1);
		else
			snd_mixer_selem_set_playback_switch_all (myData.pControledElement, 0);
	}
}



static void on_change_volume (GtkRange *range, gpointer data)
{
	double fFactor = gtk_range_get_value (GTK_RANGE (range)) / 100;
	g_print ("%s (%.2f)\n", __func__, fFactor);
	int iVolume = myData.iVolumeMin + (myData.iVolumeMax - myData.iVolumeMin) * fFactor;
	mixer_set_volume (iVolume);
}
GtkWidget *mixer_build_widget (void)
{
	g_return_val_if_fail (myData.pControledElement != NULL, NULL);
	GtkWidget *pScale = gtk_hscale_new_with_range (0., 100., 1.);
	//gtk_widget_set (pWidget, "width-request", 150, NULL);
	
	myData.iCurrentVolume = mixer_get_mean_volume ();
	gtk_range_set_value (GTK_RANGE (pScale), myData.iCurrentVolume);
	
	g_signal_connect (G_OBJECT (pScale),
		"value-changed",
		G_CALLBACK (on_change_volume),
		NULL);
	
	return pScale;
}


void mixer_show_hide_dialog (void)
{
	if (myData.pDialog == NULL)
	{
		GtkWidget *pScale = mixer_build_widget ();
		myData.pDialog = cairo_dock_build_dialog (_D("Set up volume :"),
			myIcon,
			myContainer,
			NULL,
			pScale,
			GTK_BUTTONS_NONE,
			NULL,
			NULL,
			NULL);
	}
	else
	{
		cairo_dock_dialog_unreference (myData.pDialog);
		myData.pDialog = NULL;
	}
}

gboolean mixer_check_events (gpointer data)
{
	snd_mixer_handle_events (myData.mixer_handle);
	return TRUE;
}





