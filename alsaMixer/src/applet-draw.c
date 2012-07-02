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

#include "applet-struct.h"
#include "applet-generic.h"
#include "applet-draw.h"

static void _load_mute_surface (void);



/// DRAW ICON ///

static void mixer_apply_zoom_effect (cairo_surface_t *pSurface)
{
	double fScale = .3 + .7 * myData.iCurrentVolume / 100.;
	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ZOOM (pSurface, fScale);
}

static void mixer_apply_transparency_effect (cairo_surface_t *pSurface)
{
	double fAlpha = .3 + .7 * myData.iCurrentVolume / 100.;
	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ALPHA (pSurface, fAlpha);
}

static void mixer_draw_bar (cairo_surface_t *pSurface)
{
	cd_debug ("%s (%p, %d)", __func__, pSurface, myData.iCurrentVolume);
	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR (pSurface, myData.iCurrentVolume * .01);
}

void cd_update_icon (void)
{
	gboolean bNeedRedraw = FALSE;
	
	switch (myConfig.iVolumeDisplay)
	{
		case VOLUME_NO_DISPLAY :
		break;
		
		case VOLUME_ON_LABEL :
		{
			gchar *cLabel = g_strdup_printf ("%s: %d%%", myData.mixer_card_name?myData.mixer_card_name:D_("Volume"), myData.iCurrentVolume);
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
	
	cairo_surface_t *pSurface = NULL;
	if (myConfig.iVolumeEffect != VOLUME_EFFECT_GAUGE)
	{
		if (myData.bIsMute)
		{
			if (myData.pMuteSurface == NULL)
				_load_mute_surface ();
			pSurface = myData.pMuteSurface;
		}
		else
		{
			pSurface = myData.pSurface;
		}
	}
	
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
		
		case VOLUME_EFFECT_GAUGE :
		{
			double fPercent;
			if (myData.bIsMute)
				fPercent = CAIRO_DATA_RENDERER_UNDEF_VALUE;
			else
				fPercent = (double) myData.iCurrentVolume / 100.;
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
			bNeedRedraw = FALSE;
		}
		break;
		
		default :
		break;
	}
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
	
	if (myData.pScale)
	{
		cd_mixer_set_volume_with_no_callback (myData.pScale, myData.iCurrentVolume);
	}
}


void mixer_load_surfaces (void)
{
	if (myData.pSurface != NULL)
	{
		cairo_surface_destroy (myData.pSurface);
		myData.pSurface = NULL;
	}
	
	if (myConfig.cDefaultIcon != NULL)
	{
		gchar *cImagePath = cairo_dock_search_icon_s_path (myConfig.cDefaultIcon, MAX (myIcon->iImageWidth, myIcon->iImageHeight));
		if (cImagePath == NULL)
			cImagePath = cairo_dock_search_image_s_path (myConfig.cDefaultIcon);
		if (cImagePath != NULL)
		{
			myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			g_free (cImagePath);
		}
	}
	
	if (myData.pSurface == NULL)
	{
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (MY_APPLET_SHARE_DATA_DIR"/default.svg");
	}
	
	if (myData.pMuteSurface != NULL)
	{
		cairo_surface_destroy (myData.pMuteSurface);
		myData.pMuteSurface = NULL;
	}  // don't load the mute surface now, as we often won't even need it in the session. we'll load it on demand.
}

static void _load_mute_surface (void)
{
	if (myConfig.cMuteIcon != NULL)
	{
		gchar *cImagePath = cairo_dock_search_icon_s_path (myConfig.cMuteIcon, MAX (myIcon->iImageWidth, myIcon->iImageHeight));
		if (cImagePath == NULL)
			cImagePath = cairo_dock_search_image_s_path (myConfig.cMuteIcon);
		if (cImagePath != NULL)
		{
			myData.pMuteSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cImagePath);
			g_free (cImagePath);
		}
	}
	
	if (myData.pMuteSurface == NULL)
	{
		myData.pMuteSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (MY_APPLET_SHARE_DATA_DIR"/mute.svg");
	}
}
