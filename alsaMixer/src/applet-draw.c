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
#include <string.h>
#include <glib/gi18n.h>
#include <alsa/asoundlib.h>

#include "applet-struct.h"
#include "applet-mixer.h"
#include "applet-draw.h"


int mixer_element_update_with_event (snd_mixer_elem_t *elem, unsigned int mask)
{
	CD_APPLET_ENTER;
	cd_debug ("%s (%d)", __func__, mask);
	
	if (mask != 0)
	{
		myData.iCurrentVolume = mixer_get_mean_volume ();
		myData.bIsMute = mixer_is_mute ();
		cd_debug (" iCurrentVolume <- %d bIsMute <- %d", myData.iCurrentVolume, myData.bIsMute);
	}
	gboolean bNeedRedraw = FALSE;
	
	switch (myConfig.iVolumeDisplay)
	{
		case VOLUME_NO_DISPLAY :
		break;
		
		case VOLUME_ON_LABEL :
		{
			gchar *cLabel = g_strdup_printf ("%s : %d%%", myData.mixer_card_name, myData.iCurrentVolume);
			CD_APPLET_SET_NAME_FOR_MY_ICON (cLabel);
			g_free (cLabel);
		}
		break;
		
		case VOLUME_ON_ICON :
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", myData.iCurrentVolume);
			bNeedRedraw = TRUE;
		break;
		
		default :
		break;
	}
	
	cairo_surface_t *pSurface = (myData.bIsMute ? myData.pMuteSurface : myData.pSurface);
	
	switch (myConfig.iVolumeEffect)
	{
		case VOLUME_NO_EFFECT :
			CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
			bNeedRedraw = FALSE;
		break;
		
		case VOLUME_EFFECT_ZOOM :
			mixer_apply_zoom_effect (pSurface);
			bNeedRedraw = FALSE;
		break;
		
		case VOLUME_EFFECT_TRANSPARENCY :
			mixer_apply_transparency_effect (pSurface);
			bNeedRedraw = FALSE;
		break;
		
		case VOLUME_EFFECT_BAR :
			mixer_draw_bar (pSurface);
			bNeedRedraw = FALSE;
		break;
		
		default :
		break;
	}
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
	
	if (/**myDesklet && */myData.pScale/** && mask != 0*/)
	{
		mixer_set_volume_with_no_callback (myData.pScale, myData.iCurrentVolume);
	}
	
	CD_APPLET_LEAVE();
	return 0;
}


void mixer_apply_zoom_effect (cairo_surface_t *pSurface)
{
	double fScale = .3 + .7 * myData.iCurrentVolume / 100.;
	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ZOOM (pSurface, fScale);
}

void mixer_apply_transparency_effect (cairo_surface_t *pSurface)
{
	double fAlpha = .3 + .7 * myData.iCurrentVolume / 100.;
	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ALPHA (pSurface, fAlpha);
}

void mixer_draw_bar (cairo_surface_t *pSurface)
{
	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR (pSurface, myData.iCurrentVolume * .01);
}
