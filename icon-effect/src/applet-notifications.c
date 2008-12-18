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


gboolean cd_icon_effect_start (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		pData = g_new0 (CDIconEffectData, 1);
	
	double dt = g_iGLAnimationDeltaT;
	
	if (myConfig.iFireDuration != 0)
	{
		if (pData->pFireSystem == NULL)
			pData->pFireSystem = cd_icon_effect_init_fire (pIcon, pDock, dt);
		*bStartAnimation = TRUE;
	}
	
	if (myConfig.iStarDuration != 0)
	{
		if (pData->pStarSystem == NULL)
			pData->pStarSystem = cd_icon_effect_init_stars (pIcon, pDock, dt);
		*bStartAnimation = TRUE;
	}
	
	if (myConfig.iSnowDuration != 0)
	{
		if (pData->pSnowSystem == NULL)
			pData->pSnowSystem = cd_icon_effect_init_snow (pIcon, pDock, dt);
		*bStartAnimation = TRUE;
	}
	
	if (myConfig.iRainDuration != 0)
	{
		if (pData->pRainSystem == NULL)
			pData->pRainSystem = cd_icon_effect_init_rain (pIcon, pDock, dt);
		*bStartAnimation = TRUE;
	}
	
	if (myConfig.iStormDuration != 0)
	{
		if (pData->pStormSystem == NULL)
			pData->pStormSystem = cd_icon_effect_init_storm (pIcon, pDock, dt);
		*bStartAnimation = TRUE;
	}
	
	CD_APPLET_SET_MY_ICON_DATA (pIcon, pData);
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



gboolean cd_icon_effect_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->pFireSystem != NULL)
	{
		gboolean bContinueFire = cairo_dock_update_default_particle_system (pData->pFireSystem,
		(myConfig.bContinueFire && pIcon->bPointed && pDock->bInside ? cd_icon_effect_rewind_fire_particle : NULL));
		pData->pFireSystem->fWidth = pIcon->fWidth * pIcon->fScale;
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
		(myConfig.bContinueStar && pIcon->bPointed && pDock->bInside ? cd_icon_effect_rewind_star_particle : NULL));
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
		(myConfig.bContinueSnow && pIcon->bPointed && pDock->bInside ? cd_icon_effect_rewind_snow_particle : NULL));
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
		(myConfig.bContinueRain && pIcon->bPointed && pDock->bInside ? cd_icon_effect_rewind_rain_particle : NULL));
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
		(myConfig.bContinueStorm && pIcon->bPointed && pDock->bInside ? cd_icon_effect_rewind_storm_particle : NULL));
		pData->pStormSystem->fWidth = pIcon->fWidth * pIcon->fScale;
		if (bContinueStorm)
			*bContinueAnimation = TRUE;
		else
		{
			cairo_dock_free_particle_system (pData->pStormSystem);
			pData->pStormSystem = NULL;
		}
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_icon_effect_free_data (gpointer pUserData, Icon *pIcon)
{
	g_print ("%s ()\n", __func__);
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
