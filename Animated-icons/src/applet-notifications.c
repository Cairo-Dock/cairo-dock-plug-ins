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
#include "applet-busy.h"
#include "applet-unfold.h"
#include "applet-notifications.h"

#define _REFLECT_FADE_NB_STEP 12

#define _set_new_data(icon) \
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);\
	if (pData == NULL) {\
		pData = g_new0 (CDAnimationData, 1);\
		CD_APPLET_SET_MY_ICON_DATA (pIcon, pData); }\
	else {\
		g_free (pData->pBusyImage); pData->pBusyImage = NULL;\
		g_list_foreach (pData->pUsedAnimations, (GFunc)g_free, NULL);\
		g_list_free (pData->pUsedAnimations); pData->pUsedAnimations = NULL;\
		pData->iNumRound = 0;\
		pData->bIsUnfolding = FALSE;\
		}

static int _compare_rendering_order (CDCurrentAnimation *pCurrentAnimation1, CDCurrentAnimation *pCurrentAnimation2)
{
	if (pCurrentAnimation1->pAnimation->iRenderingOrder < pCurrentAnimation2->pAnimation->iRenderingOrder)
		return -1;
	else
		return 1;
}
static void _cd_animations_start (gpointer pUserData, Icon *pIcon, CairoDock *pDock, CDAnimationsEffects *pAnimations, gboolean *bStartAnimation)
{
	_set_new_data (pIcon);
	
	gboolean bUseOpenGL = CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock));
	double dt = cairo_dock_get_animation_delta_t (CAIRO_CONTAINER (pDock));
	
	// for each animation, check if it's required.
	CDAnimationsEffects a;
	CDAnimation *pAnimation;
	int i;
	for (i = 0; pAnimations[i] < CD_ANIMATIONS_NB_EFFECTS; i ++)
	{
		a = pAnimations[i];
		pAnimation = &myData.pAnimations[a];
		CDCurrentAnimation *pCurrentAnimation = g_new0 (CDCurrentAnimation, 1);
		pCurrentAnimation->pAnimation = pAnimation;
		pCurrentAnimation->bIsPlaying = TRUE;
		pData->pUsedAnimations = g_list_insert_sorted (pData->pUsedAnimations, pCurrentAnimation, (GCompareFunc)_compare_rendering_order);
		
		if (pAnimation->init)
			pAnimation->init (pIcon, pDock, pData, dt, bUseOpenGL);
		*bStartAnimation = TRUE;
	}
}

gboolean cd_animations_on_enter (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (pIcon->bStatic || ! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)) || pIcon->iAnimationState > CAIRO_DOCK_STATE_MOUSE_HOVERED)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if (pIcon->pSubDock && pIcon->iSubdockViewType == 3 && !myDocksParam.bShowSubDockOnClick)  // icone de sous-dock avec rendu de type "box"-> on n'anime pas.
	{
		//cd_animations_free_data (pUserData, pIcon);
		return GLDI_NOTIFICATION_LET_PASS;
	}
	
	_cd_animations_start (pUserData, pIcon, pDock, myConfig.iEffectsOnMouseOver, bStartAnimation);
	
	if (bStartAnimation)
	{
		CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		pData->iNumRound = 0;
		cairo_dock_mark_icon_as_hovered_by_mouse (pIcon);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_animations_on_click (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gint iButtonState)
{
	if (! CAIRO_DOCK_IS_DOCK (pDock) || pIcon->iAnimationState > CAIRO_DOCK_STATE_CLICKED)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if (pIcon->pSubDock && pIcon->iSubdockViewType == 3)  // icone de sous-dock avec rendu de type "box" -> on arrete l'animation en cours.
	{
		CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		if (pData && ! pData->bIsUnfolding)
			cd_animations_free_data (pUserData, pIcon);  // on arrete l'animation en cours.
		return GLDI_NOTIFICATION_LET_PASS;
	}
	
	CairoDockIconGroup iType = cairo_dock_get_icon_type (pIcon);
	if (iType == CAIRO_DOCK_LAUNCHER && CAIRO_DOCK_IS_APPLI (pIcon) && ! (iButtonState & GDK_SHIFT_MASK))
		iType = CAIRO_DOCK_APPLI;
	
	gboolean bStartAnimation = FALSE;
	_cd_animations_start (pUserData, pIcon, pDock, myConfig.iEffectsOnClick[iType], &bStartAnimation);
	if (bStartAnimation)
	{
		CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		pData->iNumRound = myConfig.iNbRoundsOnClick[iType] - 1;
		cairo_dock_mark_icon_as_clicked (pIcon);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}

static inline CDAnimationsEffects _get_animation_from_name (const gchar *cName)
{
	guint iAnimationID = cairo_dock_get_animation_id (cName);
	CDAnimation *pAnimation;
	int i;
	for (i = 0; i < CD_ANIMATIONS_NB_EFFECTS; i ++)
	{
		pAnimation = &myData.pAnimations[i];
		if (pAnimation->iRegisteredId == iAnimationID)
		{
			return pAnimation->id;
		}
	}
	return -1;
}
gboolean cd_animations_on_request (gpointer pUserData, Icon *pIcon, CairoDock *pDock, const gchar *cAnimation, gint iNbRounds)
{
	if (cAnimation == NULL || pIcon == NULL || pIcon->iAnimationState > CAIRO_DOCK_STATE_CLICKED)
		return GLDI_NOTIFICATION_LET_PASS;
	
	CDAnimationsEffects anim[2] = {0, -1};
	if (strcmp (cAnimation, "default") == 0)
	{
		CairoDockIconGroup iType = cairo_dock_get_icon_type (pIcon);
		anim[0] =  myConfig.iEffectsOnClick[iType][0];
	}
	else
	{
		anim[0] = _get_animation_from_name (cAnimation);
		if (anim[0] >= CD_ANIMATIONS_NB_EFFECTS)  // enums are unsigned int, so >= 0
			return GLDI_NOTIFICATION_LET_PASS;
	}
	
	gboolean bStartAnimation = FALSE;
	_cd_animations_start (pUserData, pIcon, pDock, anim, &bStartAnimation);
	if (bStartAnimation)
	{
		CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		pData->iNumRound = iNbRounds - 1;
		cairo_dock_mark_icon_as_hovered_by_mouse (pIcon);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_animations_post_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL || pData->bIsUnfolding)
		return GLDI_NOTIFICATION_LET_PASS;
	
	CDCurrentAnimation *pCurrentAnimation;
	CDAnimation *pAnimation;
	GList *a;
	for (a = pData->pUsedAnimations; a != NULL; a = a->next)
	{
		pCurrentAnimation = a->data;
		if (pCurrentAnimation->bIsPlaying)
		{
			pAnimation = pCurrentAnimation->pAnimation;
			if (pAnimation->post_render)
			{
				pAnimation->post_render (pIcon, pDock, pData, pCairoContext);
			}
		}
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_animations_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if (pData->bIsUnfolding && pIcon->pSubDock)
	{
		if (pCairoContext != NULL)
			cd_animations_draw_unfolding_icon_cairo (pIcon, pDock, pData, pCairoContext);
		else
			cd_animations_draw_unfolding_icon (pIcon, pDock, pData);
		*bHasBeenRendered = TRUE;
		return GLDI_NOTIFICATION_LET_PASS;
	}
	
	pData->bHasBeenPulsed = FALSE;
	
	CDCurrentAnimation *pCurrentAnimation;
	CDAnimation *pAnimation;
	GList *a;
	for (a = pData->pUsedAnimations; a != NULL; a = a->next)
	{
		pCurrentAnimation = a->data;
		if (pCurrentAnimation->bIsPlaying)
		{
			pAnimation = pCurrentAnimation->pAnimation;
			if (pAnimation->render)
			{
				if (! pAnimation->bDrawIcon || ! (*bHasBeenRendered))  // if the animation draws the icon and the icon has already been drawn, skip.
				{
					pAnimation->render (pIcon, pDock, pData, pCairoContext);
					if (pAnimation->bDrawIcon)
						*bHasBeenRendered = TRUE;
				}
			}
		}
	}
	return GLDI_NOTIFICATION_LET_PASS;
}


#define _will_continue(bRepeat) ((pData->iNumRound > 0) || (pIcon->iAnimationState == CAIRO_DOCK_STATE_MOUSE_HOVERED && bRepeat && pIcon->bPointed && pDock->container.bInside) || (pIcon->iAnimationState == CAIRO_DOCK_STATE_CLICKED && myTaskbarParam.bOpeningAnimation && myTaskbarParam.bMixLauncherAppli && pIcon->iSidOpeningTimeout != 0))
gboolean cd_animations_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	gboolean bUseOpenGL = CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock));
	double dt = cairo_dock_get_animation_delta_t (CAIRO_CONTAINER (pDock));
	
	if (pData->bIsUnfolding)
	{
		if (pIcon->pSubDock->fFoldingFactor == 1 || pIcon->pSubDock == NULL || pIcon->pSubDock->icons == NULL)  // fin du pliage.
			pData->bIsUnfolding = FALSE;
		else
			*bContinueAnimation = TRUE;
		cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));  // un peu bourrin ...
		return GLDI_NOTIFICATION_LET_PASS;
	}
	
	gboolean bIconDrawn = FALSE;
	CDCurrentAnimation *pCurrentAnimation;
	CDAnimation *pAnimation;
	GList *a;
	for (a = pData->pUsedAnimations; a != NULL; a = a->next)
	{
		pCurrentAnimation = a->data;
		if (pCurrentAnimation->bIsPlaying)
		{
			pAnimation = pCurrentAnimation->pAnimation;
			if (pCurrentAnimation->bIsPlaying && pAnimation->update && (!bIconDrawn || !pAnimation->bDrawIcon))
			{
				// make 1 step
				gboolean bRepeat = _will_continue (myConfig.bContinue[pAnimation->id]);
				pCurrentAnimation->bIsPlaying = pAnimation->update (pIcon, pDock, pData, dt, bUseOpenGL, bRepeat);
				
				// go to next round if repeating
				if (! pCurrentAnimation->bIsPlaying && bRepeat)
				{
					pData->iNumRound --;
					pCurrentAnimation->bIsPlaying = TRUE;
				}
				
				// continue animation if still playing
				if (pCurrentAnimation->bIsPlaying)
				{
					if (pAnimation->bDrawIcon)
						pData->iReflectShadeCount = 0;
					*bContinueAnimation = TRUE;
				}
				else if (bUseOpenGL && pAnimation->bDrawIcon && ! pAnimation->bDrawReflect)
				{
					pData->iReflectShadeCount = _REFLECT_FADE_NB_STEP;
				}
				
				if (pAnimation->bDrawIcon)
					bIconDrawn = TRUE;
			}
		}
	}
	
	if (pData->iReflectShadeCount != 0)
	{
		pData->iReflectShadeCount --;
		pIcon->fReflectShading = (double) pData->iReflectShadeCount / _REFLECT_FADE_NB_STEP;
		if (pData->iReflectShadeCount != 0)
			*bContinueAnimation = TRUE;
		cairo_dock_redraw_icon (pIcon);
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_animations_unfold_subdock (gpointer pUserData, Icon *pIcon)  // called on start (un)folding.
{
	if (pIcon == NULL || pIcon->iSubdockViewType != 3)
		return GLDI_NOTIFICATION_LET_PASS;
	
	///CairoDock *pDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
	CairoDock *pDock = CAIRO_DOCK (cairo_dock_get_icon_container (pIcon));
	if (pDock != NULL)
	{
		_set_new_data (pIcon);
		pData->bIsUnfolding = TRUE;
		cairo_dock_launch_animation (CAIRO_CONTAINER (pDock));
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_animations_free_data (gpointer pUserData, Icon *pIcon)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	cairo_dock_free_particle_system (pData->pRaysSystem);
	
	g_free (pData->pBusyImage);  // don't delete the content as it is a copy of 'myData.pBusyImage'
	
	pIcon->fReflectShading = 0.;
	pIcon->fDeltaYReflection = 0.;
	
	g_free (pData);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	return GLDI_NOTIFICATION_LET_PASS;
}


void cd_animations_register_animation (CDAnimation *pAnimation)
{
	static int n = 0;
	pAnimation->iRenderingOrder = n;
	pAnimation->iRegisteredId = cairo_dock_register_animation (pAnimation->cName, pAnimation->cDisplayedName, FALSE);
	n ++;
}
