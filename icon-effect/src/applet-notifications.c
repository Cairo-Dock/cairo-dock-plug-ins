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

#include "applet-fire.h"
#include "applet-rain.h"
#include "applet-snow.h"
#include "applet-star.h"
#include "applet-storm.h"
#include "applet-firework.h"
#include "applet-struct.h"
#include "applet-notifications.h"


static gboolean _cd_icon_effect_start (gpointer pUserData, Icon *pIcon, CairoDock *pDock, CDIconEffectsEnum *pWantedEffects)
{
	if (! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))
		return FALSE;
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
	{
		pData = g_new0 (CDIconEffectData, 1);
		CD_APPLET_SET_MY_ICON_DATA (pIcon, pData);
	}
	double dt = cairo_dock_get_animation_delta_t (CAIRO_CONTAINER (pDock));
	
	CDIconEffectsEnum iEffect;
	CDIconEffect *pEffect;
	gboolean r, bStart = FALSE;
	int i, j=0;
	for (i = 0; i < CD_ICON_EFFECT_NB_EFFECTS; i ++)
	{
		iEffect = pWantedEffects[i];
		if (iEffect > CD_ICON_EFFECT_NB_EFFECTS - 1)
			break;
		
		pEffect = &myData.pEffects[iEffect];
		r = pEffect->init (pIcon, pDock, dt, pData);
		if (r)
		{
			bStart = TRUE;
			pData->pCurrentEffects[j++] = pEffect;
		}
	}
	return bStart;
}

gboolean cd_icon_effect_on_enter (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (pIcon->iAnimationState > CAIRO_DOCK_STATE_MOUSE_HOVERED)
		return GLDI_NOTIFICATION_LET_PASS;
	gboolean bStart = _cd_icon_effect_start (pUserData, pIcon, pDock, myConfig.iEffectsUsed);
	if (bStart)
	{
		*bStartAnimation = TRUE;
		CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		pData->iRequestTime = 0;
		cairo_dock_mark_icon_as_hovered_by_mouse (pIcon);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_icon_effect_on_click (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gint iButtonState)
{
	if (! CAIRO_DOCK_IS_DOCK (pDock) || pIcon == NULL || pIcon->iAnimationState > CAIRO_DOCK_STATE_CLICKED)
		return GLDI_NOTIFICATION_LET_PASS;
	
	CairoDockIconGroup iType = cairo_dock_get_icon_type (pIcon);
	if (iType == CAIRO_DOCK_LAUNCHER && CAIRO_DOCK_IS_APPLI (pIcon) && ! (iButtonState & GDK_SHIFT_MASK))
		iType = CAIRO_DOCK_APPLI;
	
	gboolean bStartAnimation = _cd_icon_effect_start (pUserData, pIcon, pDock, myConfig.iEffectsOnClick[iType]);
	if (bStartAnimation)
	{
		CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		pData->iRequestTime = 0;
		cairo_dock_mark_icon_as_clicked (pIcon);
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_icon_effect_on_request (gpointer pUserData, Icon *pIcon, CairoDock *pDock, const gchar *cAnimation, gint iNbRounds)
{
	if (pIcon == NULL || pIcon->iAnimationState > CAIRO_DOCK_STATE_CLICKED)
		return GLDI_NOTIFICATION_LET_PASS;
	
	CDIconEffectsEnum iEffect = -1;
	if (strcmp (cAnimation, "default") == 0)
	{
		CairoDockIconGroup iType = cairo_dock_get_icon_type (pIcon);
		iEffect = myConfig.iEffectsOnClick[iType][0];
	}
	else
	{
		int iAnimationID = cairo_dock_get_animation_id (cAnimation);
		if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_FIRE])
		{
			iEffect = CD_ICON_EFFECT_FIRE;
		}
		else if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_STARS])
		{
			iEffect = CD_ICON_EFFECT_STARS;
		}
		else if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_RAIN])
		{
			iEffect = CD_ICON_EFFECT_RAIN;
		}
		else if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_SNOW])
		{
			iEffect = CD_ICON_EFFECT_SNOW;
		}
		else if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_SAND])
		{
			iEffect = CD_ICON_EFFECT_SAND;
		}
		else if (iAnimationID == myData.iAnimationID[CD_ICON_EFFECT_FIREWORK])
		{
			iEffect = CD_ICON_EFFECT_FIREWORK;
		}
	}
	if (iEffect >= CD_ICON_EFFECT_NB_EFFECTS)
		return GLDI_NOTIFICATION_LET_PASS;
	
	CDIconEffectsEnum anim[2] = {iEffect, -1};
	int iRoundDuration = myData.pEffects[iEffect].iDuration;
	
	gboolean bStartAnimation = _cd_icon_effect_start (pUserData, pIcon, pDock, anim);
	if (bStartAnimation)
	{
		CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		pData->iRequestTime = iNbRounds * iRoundDuration;
		cairo_dock_mark_icon_as_hovered_by_mouse (pIcon);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}


static void _cd_icon_effect_render_effects (Icon *pIcon, CairoDock *pDock, CDIconEffectData *pData, gboolean bPreRender)
{
	glPushMatrix ();
	if (!pDock->container.bIsHorizontal && myConfig.bRotateEffects)
		glRotatef (pDock->container.bDirectionUp ? 90:-90, 0., 0., 1.);
	glTranslatef (0., - pIcon->fHeight * pIcon->fScale/2, 0.);  // en bas au milieu de l'icone.
	
	CDIconEffect *pEffect;
	int i;
	for (i = 0; i < CD_ICON_EFFECT_NB_EFFECTS; i ++)
	{
		pEffect = pData->pCurrentEffects[i];
		if (pEffect == NULL)
			break;
		
		if (bPreRender)
		{
			if (myConfig.bBackGround || (pEffect->render && pEffect->post_render))
				pEffect->render (pData);
		}
		else
		{
			if (!myConfig.bBackGround || (pEffect->render && pEffect->post_render))
			{
				if (pEffect->post_render)
					pEffect->post_render (pData);
				else
					pEffect->render (pData);
			}
		}
	}
	
	glPopMatrix ();
}


gboolean cd_icon_effect_pre_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, cairo_t *ctx)
{
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	_cd_icon_effect_render_effects (pIcon, pDock, pData, TRUE);
	
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_icon_effect_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	_cd_icon_effect_render_effects (pIcon, pDock, pData, FALSE);
	
	return GLDI_NOTIFICATION_LET_PASS;
}


#define _will_continue(bRepeat) ((pData->iRequestTime > 0) || (pIcon->iAnimationState == CAIRO_DOCK_STATE_MOUSE_HOVERED && bRepeat && pIcon->bPointed && pDock->container.bInside) || (pIcon->iAnimationState == CAIRO_DOCK_STATE_CLICKED && myTaskbarParam.bOpeningAnimation && myConfig.bOpeningAnimation && gldi_icon_is_launching (pIcon)))

gboolean cd_icon_effect_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if (pData->iRequestTime > 0)
	{
	        int dt = cairo_dock_get_animation_delta_t (CAIRO_CONTAINER (pDock));
		pData->iRequestTime -= dt;
		if (pData->iRequestTime < 0)
			pData->iRequestTime = 0;
	}
	
	gboolean bContinue, bRepeat;
	
	CDIconEffect *pEffect;
	int i;
	for (i = 0; i < CD_ICON_EFFECT_NB_EFFECTS; i ++)
	{
		pEffect = pData->pCurrentEffects[i];
		if (pEffect == NULL)
			break;
		
		bRepeat = _will_continue (pEffect->bRepeat);
		bContinue = pEffect->update (pIcon, pDock, bRepeat, pData);
		
		if (bContinue)
			*bContinueAnimation = TRUE;
		else
			pEffect->free (pData);
	}
	
	GdkRectangle area;
	if (pDock->container.bIsHorizontal)
	{
		area.x = pIcon->fDrawX + pIcon->fWidth * pIcon->fScale / 2 - pData->fAreaWidth/2;
		area.width = pData->fAreaWidth;
		area.height = pData->fAreaHeight;
		if (pDock->container.bDirectionUp || ! myConfig.bRotateEffects)
		{
			area.y = pIcon->fDrawY + pIcon->fHeight * pIcon->fScale + pData->fBottomGap - pData->fAreaHeight;
		}
		else
		{
			area.y = pIcon->fDrawY - pData->fBottomGap;
		}
		/*area.x = pIcon->fDrawX - .25 * pIcon->fWidth * fMaxScale;
		area.y = pIcon->fDrawY;
		area.width = pIcon->fWidth * fMaxScale * 1.5;
		area.height = pIcon->fHeight * fMaxScale + myIconsParam.iconTextDescription.iSize + 20 * fMaxScale;  // 20 = rayon max des particules, environ.
		if (pDock->container.bDirectionUp)
		{
			area.y -= myIconsParam.iconTextDescription.iSize + pIcon->fHeight * (fMaxScale - 1);
		}
		else
		{
			area.y -= 20 * fMaxScale;
		}*/
	}
	else
	{
		/*area.y = pIcon->fDrawX - .25 * pIcon->fWidth * fMaxScale;
		area.x = pIcon->fDrawY;
		area.height = pIcon->fWidth * fMaxScale * 1.5;
		area.width = pIcon->fHeight * fMaxScale + myIconsParam.iconTextDescription.iSize + 20 * fMaxScale;
		if (pDock->container.bDirectionUp)
		{
			area.x -= myIconsParam.iconTextDescription.iSize + pIcon->fHeight * (fMaxScale - 1);
		}
		else
		{
			area.x -= 20 * fMaxScale;  // rayon max des particules, environ.
		}*/
		area.y = pIcon->fDrawX + pIcon->fWidth * pIcon->fScale / 2 - pData->fAreaWidth/2;
		area.height = pData->fAreaWidth;
		area.width = pData->fAreaHeight;
		if (pDock->container.bDirectionUp || ! myConfig.bRotateEffects)
		{
			area.x = pIcon->fDrawY + pIcon->fHeight * pIcon->fScale + pData->fBottomGap - pData->fAreaHeight;
		}
		else
		{
			area.x = pIcon->fDrawY - pData->fBottomGap;
		}
	}
	if (pIcon->fOrientation == 0)
		cairo_dock_redraw_container_area (CAIRO_CONTAINER (pDock), &area);
	else
		cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));  /// il faudrait gerer la diagonale ...
	if (! *bContinueAnimation)
		cd_icon_effect_free_data (pUserData, pIcon);
	
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_icon_effect_free_data (gpointer pUserData, Icon *pIcon)
{
	cd_message ("");
	CDIconEffectData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	CDIconEffect *pEffect;
	int i;
	for (i = 0; i < CD_ICON_EFFECT_NB_EFFECTS; i ++)
	{
		pEffect = pData->pCurrentEffects[i];
		if (pEffect == NULL)
			break;
		
		pEffect->free (pData);
	}
	g_free (pData);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	return GLDI_NOTIFICATION_LET_PASS;
}
