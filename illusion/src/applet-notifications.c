/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-evaporate.h"
#include "applet-fade-out.h"
#include "applet-explode.h"
#include "applet-break.h"
#include "applet-black-hole.h"
#include "applet-notifications.h"


gboolean cd_illusion_on_remove_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock)
{
	if (! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
	{
		pData = g_new0 (CDIllusionData, 1);
		pData->fDeltaT = (double) mySystem.iGLAnimationDeltaT;
		pData->sens = (pIcon->fPersonnalScale > .05 ? 1 : -1);
		CD_APPLET_SET_MY_ICON_DATA (pIcon, pData);
		
		gboolean bSartAnimation = FALSE;
		CDIllusionEffect iEffect = (pData->sens == 1 ? myConfig.iDisappearanceEffect : myConfig.iAppearanceEffect);
		switch (iEffect)
		{
			case CD_ILLUSION_EVAPORATE :
				pData->iEffectDuration = myConfig.iEvaporateDuration;
				pData->fTimeLimitPercent = 1.;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;  // on part a rebours.
				bSartAnimation = cd_illusion_init_evaporate (pIcon, pDock, pData);
			break ;
			case CD_ILLUSION_FADE_OUT :
				pData->iEffectDuration = myConfig.iFadeOutDuration;
				pData->fTimeLimitPercent = .75;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;
				bSartAnimation = cd_illusion_init_fade_out (pIcon, pDock, pData);
			break ;
			case CD_ILLUSION_EXPLODE :
				pData->iEffectDuration = myConfig.iExplodeDuration;
				pData->fTimeLimitPercent = .9;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;
				bSartAnimation = cd_illusion_init_explode (pIcon, pDock, pData);
			break ;
			case CD_ILLUSION_BREAK :
				pData->iEffectDuration = myConfig.iBreakDuration;
				pData->fTimeLimitPercent = 1.;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;
				bSartAnimation = cd_illusion_init_break (pIcon, pDock, pData);
			break ;
			case CD_ILLUSION_BLACK_HOLE :
				pData->iEffectDuration = myConfig.iBlackHoleDuration;
				pData->fTimeLimitPercent = 1.;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;
				bSartAnimation = cd_illusion_init_black_hole (pIcon, pDock, pData);
			break ;
			default :
			break ;
		}
		if (bSartAnimation)
		{
			pData->iCurrentEffect = iEffect;
			cairo_dock_mark_icon_as_inserting_removing (pIcon);
		}
	}
	else  // si on a un pData, c'est qu'on etait deja en pleine animation, on garde la meme, qui partira en sens inverse a partir du temps actuel.
	{
		pData->sens = (pIcon->fPersonnalScale > .05 ? 1 : -1);
		cairo_dock_mark_icon_as_inserting_removing (pIcon);
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_illusion_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL || *bHasBeenRendered)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	switch (pData->iCurrentEffect)
	{
		case CD_ILLUSION_EVAPORATE :
			cd_illusion_draw_evaporate_icon (pIcon, pDock, pData);
			*bHasBeenRendered = TRUE;
		break ;
		case CD_ILLUSION_FADE_OUT :
			cd_illusion_draw_fade_out_icon (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_EXPLODE :
			cd_illusion_draw_explode_icon (pIcon, pDock, pData);
			*bHasBeenRendered = TRUE;
		break ;
		case CD_ILLUSION_BREAK :
			cd_illusion_draw_break_icon (pIcon, pDock, pData);
			*bHasBeenRendered = TRUE;
		break ;
		case CD_ILLUSION_BLACK_HOLE :
			cd_illusion_draw_black_hole_icon (pIcon, pDock, pData);
			*bHasBeenRendered = TRUE;
		break ;
		default :
		break ;
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_illusion_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	pData->fTime += pData->sens * pData->fDeltaT;
	if (pData->fTime < 0)
		pData->fTime = 0;
	switch (pData->iCurrentEffect)
	{
		case CD_ILLUSION_EVAPORATE :
			cd_illusion_update_evaporate (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_FADE_OUT :
			cd_illusion_update_fade_out (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_EXPLODE :
			cd_illusion_update_explode (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_BREAK :
			cd_illusion_update_break (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_BLACK_HOLE :
			cd_illusion_update_black_hole (pIcon, pDock, pData);
		break ;
		default :
		break ;
	}	
	
	if ((pData->sens == 1 && pData->fTime > pData->fTimeLimitPercent * pData->iEffectDuration) ||
		(pData->sens == -1 && pIcon->fPersonnalScale < -.05))
		cairo_dock_update_removing_inserting_icon_size_default (pIcon);
	
	if ((pData->sens == 1 && pData->fTime < pData->iEffectDuration) ||
		(pData->sens == -1 && pData->fTime > 0) ||
		fabs (pIcon->fPersonnalScale) > .05)
	{
		*bContinueAnimation = TRUE;
	}
	else
	{
		cd_illusion_free_data (pUserData, pIcon);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_illusion_free_data (gpointer pUserData, Icon *pIcon)
{
	cd_message ("");
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	cairo_dock_free_particle_system (pData->pEvaporateSystem);
	
	g_free (pData->pExplosionPart);
	
	g_free (pData->pBreakPart);
	
	g_free (pData->pBlackHolePoints);
	g_free (pData->pBlackHoleCoords);
	g_free (pData->pBlackHoleVertices);
	
	g_free (pData);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
