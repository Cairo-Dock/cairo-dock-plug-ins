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
#include <math.h>

#include "applet-rotation.h"
#include "applet-spot.h"
#include "applet-struct.h"
#include "applet-rays.h"
#include "applet-wobbly.h"
#include "applet-mesh-factory.h"
#include "applet-wave.h"
#include "applet-pulse.h"
#include "applet-bounce.h"
#include "applet-blink.h"
#include "applet-unfold.h"
#include "applet-notifications.h"

#define _REFLECT_FADE_NB_STEP 12

#define _set_new_data(icon) \
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);\
	if (pData == NULL) {\
		pData = g_new0 (CDAnimationData, 1);\
		CD_APPLET_SET_MY_ICON_DATA (pIcon, pData); }\
	else {\
		pData->fRadiusFactor = 0;\
		pData->bIsWobblying = FALSE;\
		pData->bIsWaving = FALSE;\
		pData->fPulseAlpha = 0;\
		pData->bIsBouncing = FALSE;\
		pData->bIsBlinking = FALSE;\
		pData->iNumRound = 0;\
		pData->bIsUnfolding = FALSE; }

static void _cd_animations_start (gpointer pUserData, Icon *pIcon, CairoDock *pDock, CDAnimationsEffects *pAnimations, gboolean *bStartAnimation)
{
	_set_new_data (pIcon);
	
	gboolean bUseOpenGL = CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock));
	double dt = (bUseOpenGL ? mySystem.iGLAnimationDeltaT : mySystem.iCairoAnimationDeltaT);
	
	int i;
	for (i = 0; i < CD_ANIMATIONS_NB_EFFECTS; i ++)
	{
		switch (pAnimations[i])
		{
			case CD_ANIMATIONS_BOUNCE :
				cd_animations_init_bounce (pDock, pData, dt);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ANIMATIONS_ROTATE :
				cd_animations_init_rotation (pData, dt, bUseOpenGL);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ANIMATIONS_BLINK :
				cd_animations_init_blink (pData, dt);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ANIMATIONS_PULSE :
				cd_animations_init_pulse (pData, dt);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ANIMATIONS_WOBBLY :
				cd_animations_init_wobbly (pData, bUseOpenGL);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ANIMATIONS_WAVE :
				if (! bUseOpenGL)
					break ;
				cd_animations_init_wave (pData);
				*bStartAnimation = TRUE;
			break;
			
			case CD_ANIMATIONS_SPOT :
				if (! bUseOpenGL)
					break ;
				cd_animations_init_spot (pIcon, pDock, pData, dt);
				*bStartAnimation = TRUE;
			break;
			
			default :
				i = CD_ANIMATIONS_NB_EFFECTS - 1;
			break;
		}
	}
	if (pData->fRadiusFactor == 0)
		pData->fIconOffsetY = 0;
}

gboolean cd_animations_on_enter (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (pIcon->bStatic || ! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)) || pIcon->iAnimationState > CAIRO_DOCK_STATE_MOUSE_HOVERED)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pIcon->pSubDock && pIcon->iSubdockViewType == 3 && !myAccessibility.bShowSubDockOnClick)
	{
		//cd_animations_free_data (pUserData, pIcon);
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	_cd_animations_start (pUserData, pIcon, pDock, myConfig.iEffectsOnMouseOver, bStartAnimation);
	
	if (bStartAnimation)
	{
		CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		pData->iNumRound = 0;
		cairo_dock_mark_icon_as_hovered_by_mouse (pIcon);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_animations_on_click (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gint iButtonState)
{
	if (! CAIRO_DOCK_IS_DOCK (pDock) || pIcon->iAnimationState > CAIRO_DOCK_STATE_CLICKED)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pIcon->pSubDock && pIcon->iSubdockViewType == 3)
	{
		CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		if (pData && ! pData->bIsUnfolding)
			cd_animations_free_data (pUserData, pIcon);  // on arrete l'animation en cours.
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	CairoDockIconGroup iType = cairo_dock_get_icon_type (pIcon);
	if (iType == CAIRO_DOCK_LAUNCHER && CAIRO_DOCK_IS_APPLI (pIcon) && ! (iButtonState & GDK_SHIFT_MASK))
		iType = CAIRO_DOCK_APPLI;
	/**if (iType == CAIRO_DOCK_APPLI && CAIRO_DOCK_IS_LAUNCHER (pIcon) && iButtonState & GDK_SHIFT_MASK)
		iType = CAIRO_DOCK_LAUNCHER;*/
	
	gboolean bStartAnimation = FALSE;
	_cd_animations_start (pUserData, pIcon, pDock, myConfig.iEffectsOnClick[iType], &bStartAnimation);
	if (bStartAnimation)
	{
		CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		pData->iNumRound = myConfig.iNbRoundsOnClick[iType] - 1;
		cairo_dock_mark_icon_as_clicked (pIcon);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_animations_on_request (gpointer pUserData, Icon *pIcon, CairoDock *pDock, const gchar *cAnimation, gint iNbRounds)
{
	if (cAnimation == NULL || pIcon == NULL || pIcon->iAnimationState > CAIRO_DOCK_STATE_CLICKED)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CDAnimationsEffects anim[2] = {0, -1};
	if (strcmp (cAnimation, "default") == 0)
	{
		CairoDockIconGroup iType = cairo_dock_get_icon_type (pIcon);
		anim[0] =  myConfig.iEffectsOnClick[iType][0];
	}
	else
	{
		int iAnimationID = cairo_dock_get_animation_id (cAnimation);
		if (iAnimationID == myData.iAnimationID[CD_ANIMATIONS_BOUNCE])
			anim[0] = CD_ANIMATIONS_BOUNCE;
		else if (iAnimationID == myData.iAnimationID[CD_ANIMATIONS_ROTATE])
			anim[0] = CD_ANIMATIONS_ROTATE;
		else if (iAnimationID == myData.iAnimationID[CD_ANIMATIONS_BLINK])
			anim[0] = CD_ANIMATIONS_BLINK;
		else if (iAnimationID == myData.iAnimationID[CD_ANIMATIONS_PULSE])
			anim[0] = CD_ANIMATIONS_PULSE;
		else if (iAnimationID == myData.iAnimationID[CD_ANIMATIONS_WOBBLY])
			anim[0] = CD_ANIMATIONS_WOBBLY;
		else if (iAnimationID == myData.iAnimationID[CD_ANIMATIONS_WAVE])
			anim[0] = CD_ANIMATIONS_WAVE;
		else if (iAnimationID == myData.iAnimationID[CD_ANIMATIONS_SPOT])
			anim[0] = CD_ANIMATIONS_SPOT;
		else
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	gboolean bStartAnimation = FALSE;
	_cd_animations_start (pUserData, pIcon, pDock, anim, &bStartAnimation);
	
	if (bStartAnimation)
	{
		CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		pData->iNumRound = iNbRounds - 1;
		cairo_dock_mark_icon_as_hovered_by_mouse (pIcon);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


static void _cd_animations_render_rays (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, int iDepth)
{
	glPushMatrix ();
	if (pDock->container.bIsHorizontal)
		glTranslatef (0., - pIcon->fHeight * pIcon->fScale/2, 0.);
	else
		glTranslatef (- pIcon->fHeight * pIcon->fScale/2, 0., 0.);
	
	if (! pDock->container.bIsHorizontal)
		glRotatef (-90, 0., 0., 1.);
	
	if (pData->pRaysSystem != NULL)
	{
		cairo_dock_render_particles_full (pData->pRaysSystem, iDepth);
	}

	glPopMatrix ();
}
gboolean cd_animations_post_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL || pData->bIsUnfolding)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->bIsBouncing)
	{
		if (pCairoContext != NULL)
			cd_animations_draw_bounce_cairo (pIcon, pDock, pData, pCairoContext, -1);
		else
			cd_animations_draw_bounce_icon (pIcon, pDock, pData, -1);
	}
	
	if (pData->bIsBlinking)
	{
		cd_animations_draw_blink_icon (pIcon, pDock, pData, -1);
	}
	
	if (pData->fRadiusFactor != 0)
	{
		if (pDock->container.bIsHorizontal)
			glTranslatef (0., - pData->fIconOffsetY * (pDock->container.bDirectionUp ? 1 : -1), 0.);
		else
			glTranslatef (- pData->fIconOffsetY * (pDock->container.bDirectionUp ? -1 : 1), 0., 0.);
		if (pData->pRaysSystem != NULL)
			_cd_animations_render_rays (pIcon, pDock, pData, 1);
		
		cd_animation_render_spot_front (pIcon, pDock, pData->fRadiusFactor);
		if (pData->fHaloRotationAngle > 90 && pData->fHaloRotationAngle < 270)
			cd_animation_render_halo (pIcon, pDock, pData->fRadiusFactor, pData->fHaloRotationAngle);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_animations_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->bIsUnfolding && pIcon->pSubDock)
	{
		if (pCairoContext != NULL)
			cd_animations_draw_unfolding_icon_cairo (pIcon, pDock, pData, pCairoContext);
		else
			cd_animations_draw_unfolding_icon (pIcon, pDock, pData);
		*bHasBeenRendered = TRUE;
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	gboolean bHasBeenPulsed = FALSE;
	if (*bHasBeenRendered)
	{
		if (pData->fPulseAlpha != 0)
		{
			if (pCairoContext != NULL)
				cd_animations_draw_pulse_cairo (pIcon, pDock, pData, pCairoContext);
			else
				cd_animations_draw_pulse_icon (pIcon, pDock, pData);
		}
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	if (pData->bIsBlinking)
	{
		cd_animations_draw_blink_icon (pIcon, pDock, pData, 1);
	}
	
	if (pData->fRadiusFactor != 0)
	{
		cd_animation_render_spot (pIcon, pDock, pData->fRadiusFactor);
		if (pData->fHaloRotationAngle <= 90 || pData->fHaloRotationAngle >= 270)
			cd_animation_render_halo (pIcon, pDock, pData->fRadiusFactor, pData->fHaloRotationAngle);
		
		if (pData->pRaysSystem != NULL)
			_cd_animations_render_rays (pIcon, pDock, pData, 1);
		
		if (pDock->container.bIsHorizontal)
			glTranslatef (0., pData->fIconOffsetY * (pDock->container.bDirectionUp ? 1 : -1), 0.);
		else
			glTranslatef (pData->fIconOffsetY * (pDock->container.bDirectionUp ? -1 : 1), 0., 0.);
	}
	
	if (pData->bIsBouncing)
	{
		if (pCairoContext != NULL)
			cd_animations_draw_bounce_cairo (pIcon, pDock, pData, pCairoContext, 1);
		else
			cd_animations_draw_bounce_icon (pIcon, pDock, pData, 1);
	}
	
	if (pData->bIsWobblying)
	{
		if (pCairoContext != NULL)
			cd_animations_draw_wobbly_cairo (pIcon, pDock, pData, pCairoContext);
		else
			cd_animations_draw_wobbly_icon (pIcon, pDock, pData);
		*bHasBeenRendered = TRUE;
	}
	else if (pData->bIsWaving)
	{
		cd_animations_draw_wave_icon (pIcon, pDock, pData);
		*bHasBeenRendered = TRUE;
	}
	else if (pData->fRotationSpeed != 0)
	{
		if (pCairoContext != NULL)
			cd_animations_draw_rotating_cairo (pIcon, pDock, pData, pCairoContext);
		else
		{
			cd_animations_draw_rotating_icon (pIcon, pDock, pData);
			bHasBeenPulsed = myConfig.bPulseSameShape;
		}
		*bHasBeenRendered = TRUE;
	}
	
	if (pData->fPulseAlpha != 0 && ! bHasBeenPulsed)
	{
		if (pCairoContext != NULL)
			cd_animations_draw_pulse_cairo (pIcon, pDock, pData, pCairoContext);
		else
			cd_animations_draw_pulse_icon (pIcon, pDock, pData);
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


#define _will_continue(bRepeat) ((pData->iNumRound > 0) || (pIcon->iAnimationState == CAIRO_DOCK_STATE_MOUSE_HOVERED && bRepeat && pIcon->bPointed && pDock->container.bInside))
gboolean cd_animations_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	gboolean bUseOpenGL = CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock));
	double dt = (bUseOpenGL ? mySystem.iGLAnimationDeltaT : mySystem.iCairoAnimationDeltaT);
	
	if (pData->bIsUnfolding)
	{
		if (pIcon->pSubDock->fFoldingFactor == 1 || pIcon->pSubDock == NULL || pIcon->pSubDock->icons == NULL)  // fin du pliage.
			pData->bIsUnfolding = FALSE;
		else
			*bContinueAnimation = TRUE;
		cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));  // un peu bourrin ...
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	if (pData->bIsWobblying)
	{
		if (bUseOpenGL)
			pData->bIsWobblying = cd_animations_update_wobbly (pDock, pData, dt, _will_continue (myConfig.bContinueWobbly));
		else
			pData->bIsWobblying = cd_animations_update_wobbly_cairo (pIcon, pDock, pData, _will_continue (myConfig.bContinueWobbly));
		
		if (! pData->bIsWobblying && _will_continue (myConfig.bContinueWobbly))
		{
			pData->iNumRound --;
			cd_animations_init_wobbly (pData, bUseOpenGL);
		}
		if (pData->bIsWobblying)
		{
			pData->iReflectShadeCount = 0;
			*bContinueAnimation = TRUE;
		}
		else if (bUseOpenGL)
			pData->iReflectShadeCount = _REFLECT_FADE_NB_STEP;
	}
	if (! pData->bIsWobblying && pData->bIsWaving)
	{
		pData->bIsWaving = cd_animations_update_wave (pDock, pData, dt);
		if (! pData->bIsWaving && _will_continue (myConfig.bContinueWave))
		{
			pData->iNumRound --;
			pData->bIsWaving = TRUE;
			pData->fWavePosition = - myConfig.fWaveWidth / 2;
		}
		if (pData->bIsWaving)
		{
			pData->iReflectShadeCount = 0;
			*bContinueAnimation = TRUE;
		}
		else if (bUseOpenGL)
			pData->iReflectShadeCount = _REFLECT_FADE_NB_STEP;
	}
	if (! pData->bIsWobblying && ! pData->bIsWaving && pData->fRotationSpeed != 0)
	{
		cd_animations_update_rotating (pIcon, pDock, pData, bUseOpenGL, _will_continue (myConfig.bContinueRotation));
		if (pData->fRotationAngle < 360)
		{
			pData->iReflectShadeCount = 0;
			*bContinueAnimation = TRUE;
		}
		else
		{
			if (_will_continue (myConfig.bContinueRotation))
			{
				pData->fRotationAngle -= 360;
				pData->iNumRound --;
				pData->iReflectShadeCount = 0;
				*bContinueAnimation = TRUE;
			}
			else
			{
				pData->fRotationAngle = 0;
				pData->fRotationSpeed = 0;
				if (bUseOpenGL)
					pData->iReflectShadeCount = _REFLECT_FADE_NB_STEP;
			}
		}
	}
	
	if (pData->iReflectShadeCount != 0)
	{
		pData->iReflectShadeCount --;
		pIcon->fReflectShading = (double) pData->iReflectShadeCount / _REFLECT_FADE_NB_STEP;
		if (pData->iReflectShadeCount != 0)
			*bContinueAnimation = TRUE;
	}
	
	if (pData->fRadiusFactor != 0)
	{
		gboolean bContinueSpot = cd_animations_update_spot (pIcon, pDock, pData, dt, _will_continue (myConfig.bContinueSpot));
		if (bContinueSpot)  // l'animation doit continuer, qu'on ait passe un tour ou pas.
			*bContinueAnimation = TRUE;
		if (pData->fHaloRotationAngle > 360)  // un tour est passe.
		{
			pData->fHaloRotationAngle -= 360;
			if (pData->iNumRound > 0)
			{
				pData->iNumRound --;
			}
		}
	}
	
	if (pData->fPulseAlpha != 0)
	{
		gboolean bContinuePulse = cd_animations_update_pulse (pIcon, pDock, pData, bUseOpenGL);
		if (bContinuePulse)
			*bContinueAnimation = TRUE;
		else if (_will_continue (myConfig.bContinuePulse))
		{
			pData->iNumRound --;
			cd_animations_init_pulse (pData, dt);
			*bContinueAnimation = TRUE;
		}
	}
	
	if (pData->bIsBouncing)
	{
		pData->bIsBouncing = cd_animations_update_bounce (pIcon, pDock, pData, dt, bUseOpenGL, _will_continue (myConfig.bContinueBounce));
		if (! pData->bIsBouncing && _will_continue (myConfig.bContinueBounce))
		{
			pData->iNumRound --;
			cd_animations_init_bounce (pDock, pData, dt);
		}
		if (pData->bIsBouncing)
			*bContinueAnimation = TRUE;
	}
	
	if (pData->bIsBlinking)
	{
		pData->bIsBlinking = cd_animations_update_blink (pIcon, pDock, pData, dt, bUseOpenGL);
		if (! pData->bIsBlinking && _will_continue (myConfig.bContinueBlink))
		{
			pData->iNumRound --;
			cd_animations_init_blink (pData, dt);
		}
		if (pData->bIsBlinking)
			*bContinueAnimation = TRUE;
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_animations_unfold_subdock (gpointer pUserData, Icon *pIcon)  // called on start (un)folding.
{
	if (pIcon == NULL || pIcon->iSubdockViewType != 3)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CairoDock *pDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
	if (pDock != NULL)
	{
		_set_new_data (pIcon);
		pData->bIsUnfolding = TRUE;
		cairo_dock_launch_animation (CAIRO_CONTAINER (pDock));
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_animations_free_data (gpointer pUserData, Icon *pIcon)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	cairo_dock_free_particle_system (pData->pRaysSystem);
	
	pIcon->fReflectShading = 0.;
	pIcon->fDeltaYReflection = 0.;
	
	g_free (pData);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
