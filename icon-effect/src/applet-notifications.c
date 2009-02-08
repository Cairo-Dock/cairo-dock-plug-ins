/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-fire.h"
#include "applet-rain.h"
#include "applet-snow.h"
#include "applet-star.h"
#include "applet-storm.h"
#include "applet-struct.h"
#include "applet-notifications.h"


static void _cd_icon_effect_start (gpointer pUserData, Icon *pIcon, CairoDock *pDock, CDIconEffects *pAnimations, gboolean *bStartAnimation)
{
	if (! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))
		return ;
	
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
	{
		pData = g_new0 (CDIconEffectData, 1);
		CD_APPLET_SET_MY_ICON_DATA (pIcon, pData);
	}
	double dt = mySystem.iGLAnimationDeltaT;
	
	int i;
	for (i = 0; i < CD_ICON_EFFECT_NB_EFFECTS; i ++)
	{
		switch (pAnimations[i])
		{
			case CD_ICON_EFFECT_FIRE :
				if (pData->pFireSystem == NULL)
					pData->pFireSystem = cd_icon_effect_init_fire (pIcon, pDock, dt);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ICON_EFFECT_STARS :
				if (pData->pStarSystem == NULL)
					pData->pStarSystem = cd_icon_effect_init_stars (pIcon, pDock, dt);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ICON_EFFECT_RAIN :
				if (pData->pRainSystem == NULL)
					pData->pRainSystem = cd_icon_effect_init_rain (pIcon, pDock, dt);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ICON_EFFECT_SNOW :
				if (pData->pSnowSystem == NULL)
					pData->pSnowSystem = cd_icon_effect_init_snow (pIcon, pDock, dt);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ICON_EFFECT_SAND :
				if (pData->pStormSystem == NULL)
					pData->pStormSystem = cd_icon_effect_init_storm (pIcon, pDock, dt);
				*bStartAnimation = TRUE;
			break;
			
			default :
				i = CD_ICON_EFFECT_NB_EFFECTS - 1;
			break;
		}
	}
}

gboolean cd_icon_effect_on_enter (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (pIcon->iAnimationState > CAIRO_DOCK_STATE_MOUSE_HOVERED)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	_cd_icon_effect_start (pUserData, pIcon, pDock, myConfig.iEffectsUsed, bStartAnimation);
	if (bStartAnimation)
		cairo_dock_mark_icon_as_hovered_by_mouse (pIcon);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_icon_effect_on_click (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gint iButtonState)
{
	if (! CAIRO_DOCK_IS_DOCK (pDock) || pIcon->iAnimationState > CAIRO_DOCK_STATE_CLICKED)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CairoDockIconType iType = cairo_dock_get_icon_type (pIcon);
	if (iType == CAIRO_DOCK_APPLI && CAIRO_DOCK_IS_LAUNCHER (pIcon) && iButtonState & GDK_SHIFT_MASK)
		iType = CAIRO_DOCK_LAUNCHER;
	
	gboolean bStartAnimation = FALSE;
	_cd_icon_effect_start (pUserData, pIcon, pDock, myConfig.iEffectsOnClick[iType], &bStartAnimation);
	if (bStartAnimation)
		cairo_dock_mark_icon_as_clicked (pIcon);
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_icon_effect_on_request (gpointer pUserData, Icon *pIcon, CairoDock *pDock, const gchar *cAnimation, gint iNbRounds)
{
	if (! CAIRO_DOCK_IS_DOCK (pDock) || pIcon->iAnimationState > CAIRO_DOCK_STATE_CLICKED)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CDIconEffects anim[2] = {0, -1};
	
	if (strcmp (cAnimation, "default") == 0)
	{
		CairoDockIconType iType = cairo_dock_get_icon_type (pIcon);
		anim[0] = myConfig.iEffectsOnClick[iType][0];
	}
	else
	{
		int iAnimationID = cairo_dock_get_animation_id (cAnimation);
		if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_FIRE])
			anim[0] = CD_ICON_EFFECT_FIRE;
		else if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_STARS])
			anim[0] = CD_ICON_EFFECT_STARS;
		else if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_RAIN])
			anim[0] = CD_ICON_EFFECT_RAIN;
		else if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_SNOW])
			anim[0] = CD_ICON_EFFECT_SNOW;
		else if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_SAND])
			anim[0] = CD_ICON_EFFECT_SAND;
		else
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	gboolean bStartAnimation = FALSE;
	_cd_icon_effect_start (pUserData, pIcon, pDock, anim, &bStartAnimation);
	if (bStartAnimation)
		cairo_dock_mark_icon_as_hovered_by_mouse (pIcon);
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


static void _cd_icon_effect_render_effects (Icon *pIcon, CairoDock *pDock, CDIconEffectData *pData)
{
	glPushMatrix ();
	if (!pDock->bHorizontalDock && myConfig.bRotateEffects)
		glRotatef (pDock->bDirectionUp ? 90:-90, 0., 0., 1.);
	glTranslatef (0., - pIcon->fHeight * pIcon->fScale/2, 0.);
	
	if (pData->pFireSystem != NULL)
	{
		cairo_dock_render_particles (pData->pFireSystem);
	}
	
	if (pData->pStarSystem != NULL)
	{
		cairo_dock_render_particles (pData->pStarSystem);
	}
	
	if (pData->pSnowSystem != NULL)
	{
		cairo_dock_render_particles (pData->pSnowSystem);
	}
	
	if (pData->pRainSystem != NULL)
	{
		cairo_dock_render_particles (pData->pRainSystem);
	}
	
	glPopMatrix ();
}

static void _cd_icon_effect_render_effects_with_depth (Icon *pIcon, CairoDock *pDock, CDIconEffectData *pData, int iDepth)
{
	glPushMatrix ();
	if (!pDock->bHorizontalDock && myConfig.bRotateEffects)
		glRotatef (pDock->bDirectionUp ? 90:-90, 0., 0., 1.);
	glTranslatef (0., - pIcon->fHeight * pIcon->fScale/2, 0.);
	
	if (pData->pStormSystem != NULL)
	{
		cairo_dock_render_particles_full (pData->pStormSystem, iDepth);
	}
	
	glPopMatrix ();
}

gboolean cd_icon_effect_pre_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock)
{
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (myConfig.bBackGround)
		_cd_icon_effect_render_effects (pIcon, pDock, pData);
	
	if (pData->pStormSystem != NULL)
		_cd_icon_effect_render_effects_with_depth (pIcon, pDock, pData, -1);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_icon_effect_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (! myConfig.bBackGround)
		_cd_icon_effect_render_effects (pIcon, pDock, pData);
	
	if (pData->pStormSystem != NULL)
		_cd_icon_effect_render_effects_with_depth (pIcon, pDock, pData, 1);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


#define _will_continue(bRepeat) (pIcon->iAnimationState == CAIRO_DOCK_STATE_MOUSE_HOVERED && bRepeat && pIcon->bPointed && pDock->bInside)

gboolean cd_icon_effect_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->pFireSystem != NULL)
	{
		gboolean bContinueFire = cd_icon_effect_update_fire_system (pData->pFireSystem,
		(_will_continue (myConfig.bContinueFire) ? cd_icon_effect_rewind_fire_particle : NULL));
		pData->pFireSystem->fWidth = pIcon->fWidth * pIcon->fScale;
		if (pDock->bAtBottom)
			pData->pFireSystem->fHeight = pIcon->fHeight;
		if (bContinueFire)
			*bContinueAnimation = TRUE;
		else
		{
			cairo_dock_free_particle_system (pData->pFireSystem);
			pData->pFireSystem = NULL;
		}
	}
	
	if (pData->pStarSystem != NULL)
	{
		gboolean bContinueStar = cd_icon_effect_update_star_system (pData->pStarSystem,
		(_will_continue (myConfig.bContinueStar) ? cd_icon_effect_rewind_star_particle : NULL));
		pData->pStarSystem->fWidth = pIcon->fWidth * pIcon->fScale;
		if (bContinueStar)
			*bContinueAnimation = TRUE;
		else
		{
			cairo_dock_free_particle_system (pData->pStarSystem);
			pData->pStarSystem = NULL;
		}
	}
	
	if (pData->pSnowSystem != NULL)
	{
		gboolean bContinueSnow = cd_icon_effect_update_snow_system (pData->pSnowSystem,
		(_will_continue (myConfig.bContinueSnow) ? cd_icon_effect_rewind_snow_particle : NULL));
		pData->pSnowSystem->fWidth = pIcon->fWidth * pIcon->fScale;
		if (bContinueSnow)
			*bContinueAnimation = TRUE;
		else
		{
			cairo_dock_free_particle_system (pData->pSnowSystem);
			pData->pSnowSystem = NULL;
		}
	}
	
	if (pData->pRainSystem != NULL)
	{
		gboolean bContinueRain = cd_icon_effect_update_rain_system (pData->pRainSystem,
		(_will_continue (myConfig.bContinueRain) ? cd_icon_effect_rewind_rain_particle : NULL));
		pData->pRainSystem->fWidth = pIcon->fWidth * pIcon->fScale;
		if (bContinueRain)
			*bContinueAnimation = TRUE;
		else
		{
			cairo_dock_free_particle_system (pData->pRainSystem);
			pData->pRainSystem = NULL;
		}
	}
	
	if (pData->pStormSystem != NULL)
	{
		gboolean bContinueStorm = cd_icon_effect_update_storm_system (pData->pStormSystem,
		(_will_continue (myConfig.bContinueStorm) ? cd_icon_effect_rewind_storm_particle : NULL));
		pData->pStormSystem->fWidth = pIcon->fWidth * pIcon->fScale;
		if (bContinueStorm)
			*bContinueAnimation = TRUE;
		else
		{
			cairo_dock_free_particle_system (pData->pStormSystem);
			pData->pStormSystem = NULL;
		}
	}
	
	double fMaxScale = cairo_dock_get_max_scale (pDock);
	GdkRectangle area;
	if (pDock->bHorizontalDock)
	{
		area.x = pIcon->fDrawX - .2 * pIcon->fWidth * pIcon->fScale;
		area.y = pIcon->fDrawY;
		if (pDock->bDirectionUp)
			area.y -= (pIcon->fScale - fMaxScale) * pIcon->fHeight + myLabels.iconTextDescription.iSize;
		area.width = pIcon->fWidth * pIcon->fScale * 1.4;
		area.height = pIcon->fHeight * fMaxScale + myLabels.iconTextDescription.iSize;
	}
	else
	{
		area.y = pIcon->fDrawX - .2 * pIcon->fWidth * pIcon->fScale;
		area.x = pIcon->fDrawY;
		if (pDock->bDirectionUp)
			area.x -= (pIcon->fScale - fMaxScale) * pIcon->fHeight + myLabels.iconTextDescription.iSize;
		area.height = pIcon->fWidth * pIcon->fScale * 1.4;
		area.width = pIcon->fHeight * fMaxScale + myLabels.iconTextDescription.iSize;
	}
	cairo_dock_redraw_container_area (pDock, &area);
	
	if (! *bContinueAnimation)
		cd_icon_effect_free_data (pUserData, pIcon);
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_icon_effect_free_data (gpointer pUserData, Icon *pIcon)
{
	cd_message ("");
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->pFireSystem != NULL)
		cairo_dock_free_particle_system (pData->pFireSystem);
	
	if (pData->pStarSystem != NULL)
		cairo_dock_free_particle_system (pData->pStarSystem);
	
	if (pData->pRainSystem != NULL)
		cairo_dock_free_particle_system (pData->pRainSystem);
	
	if (pData->pSnowSystem != NULL)
		cairo_dock_free_particle_system (pData->pSnowSystem);
	
	if (pData->pStormSystem != NULL)
		cairo_dock_free_particle_system (pData->pStormSystem);
	
	g_free (pData);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
