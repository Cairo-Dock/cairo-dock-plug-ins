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
#include "applet-mixer.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS



int mixer_element_update_with_event (snd_mixer_elem_t *elem, unsigned int mask)
{
	cd_debug ("%s (%d)", __func__, mask);
	
	if (mask != 0)
	{
		myData.iCurrentVolume = mixer_get_mean_volume ();
		myData.bIsMute = mixer_is_mute ();
		cd_debug (" iCurrentVolume <- %d bIsMute <- %d", myData.iCurrentVolume, myData.bIsMute);
	}
	
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
	
	cairo_surface_t *pSurface = (myData.bIsMute ? myData.pMuteSurface : myData.pSurface);
	
	switch (myConfig.iVolumeEffect)
	{
		case VOLUME_NO_EFFECT :
			CD_APPLET_SET_SURFACE_ON_MY_ICON (pSurface);
		break;
		
		case VOLUME_EFFECT_ZOOM :
			mixer_apply_zoom_effect (pSurface);
		break;
		
		case VOLUME_EFFECT_TRANSPARENCY :
			mixer_apply_transparency_effect (pSurface);
		break;
		
		case VOLUME_EFFECT_BAR :
			mixer_draw_bar (pSurface);
		break;
		
		default :
		break;
	}
	
	if (myDesklet && myData.pScale && mask != 0)
	{
		mixer_set_volume_with_no_callback (myData.pScale, myData.iCurrentVolume);
	}
	
	return 0;
}


void mixer_apply_zoom_effect (cairo_surface_t *pSurface)
{
	cairo_save (myDrawContext);
	double fScale = .3 + .7 * myData.iCurrentVolume / 100.;
	/*double fMaxScale = (myDock ? 1 + g_fAmplitude : 1);
	cairo_translate (myDrawContext, myIcon->fWidth * fMaxScale / 2 * (1 - fScale) , myIcon->fHeight * fMaxScale / 2 * (1 - fScale));
	cairo_scale (myDrawContext, fScale, fScale);
	cairo_dock_set_icon_surface_with_reflect (myDrawContext, pSurface, myIcon, myContainer);  // on n'utilise pas la macro car on ne veut pas du redraw.*/
	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ZOOM (pSurface, fScale)
	cairo_restore (myDrawContext);
}

void mixer_apply_transparency_effect (cairo_surface_t *pSurface)
{
	cd_debug ("%s (%x)", __func__, pSurface);
	cairo_save (myDrawContext);
	/*cairo_set_source_rgba (myDrawContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	if (pSurface != NULL)
	{
		cairo_set_source_surface (
			myDrawContext,
			pSurface,
			0.,
			0.);
		double fAlpha = .3 + .7 * myData.iCurrentVolume / 100.;
		cairo_paint_with_alpha (myDrawContext, fAlpha);
	}*/
	double fAlpha = .3 + .7 * myData.iCurrentVolume / 100.;
	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_ALPHA (pSurface, fAlpha)
	cairo_restore (myDrawContext);
}

void mixer_draw_bar (cairo_surface_t *pSurface)
{
	cairo_save (myDrawContext);
	cairo_dock_set_icon_surface_with_reflect (myDrawContext, pSurface, myIcon, myContainer);  // on n'utilise pas la macro car on ne veut pas du redraw.
	
	cairo_restore (myDrawContext);
	cairo_save (myDrawContext);
	/*double fMaxScale = (myDock ? 1 + g_fAmplitude : 1);
	cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
		0.,
		myIcon->fWidth * fMaxScale,
		0.);  // de gauche a droite.
	g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
	
	cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		0.,
		1.,
		0.,
		0.,
		1.);
	cairo_pattern_add_color_stop_rgba (pGradationPattern,
		1.,
		0.,
		1.,
		0.,
		1.);
	
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	cairo_set_line_width (myDrawContext, 6);
	cairo_set_line_cap (myDrawContext, CAIRO_LINE_CAP_ROUND);
	
	cairo_move_to (myDrawContext, 3, myIcon->fHeight * fMaxScale - 3);
	cairo_rel_line_to (myDrawContext, (myIcon->fWidth * fMaxScale - 6) * myData.iCurrentVolume * .01, 0);
	
	cairo_set_source (myDrawContext, pGradationPattern);
	cairo_stroke (myDrawContext);
	
	cairo_pattern_destroy (pGradationPattern);
	*/
	CD_APPLET_SET_SURFACE_ON_MY_ICON_WITH_BAR(pSurface, myData.iCurrentVolume * .01)
	cairo_restore (myDrawContext);
	
	CD_APPLET_REDRAW_MY_ICON
}
