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

#include "applet-struct.h"
#include "applet-notifications.h"

#define CD_DOCK_IN_MOVMENT(pDock) ((myConfig.bAlways && cairo_dock_container_is_animating (CAIRO_CONTAINER(pDock))) || pDock->bIsShrinkingDown || pDock->bIsGrowingUp)


gboolean cd_motion_blur_pre_render (gpointer pUserData, CairoDock *pDock, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	CDMotionBlurData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	
	if ((pData != NULL && pData->iBlurCount != 0) || CD_DOCK_IN_MOVMENT (pDock))
		glAccum(GL_MULT, myConfig.fBlurFactor);
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_motion_blur_post_render (gpointer pUserData, CairoDock *pDock, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	CDMotionBlurData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	
	if ((pData != NULL && pData->iBlurCount != 0) || CD_DOCK_IN_MOVMENT (pDock))
	{
		glAccum (GL_ACCUM, 1 - myConfig.fBlurFactor);
		glAccum (GL_RETURN, 1.0);
	}
	else
	{
		glClearAccum (0., 0., 0., 0.);
		glClear (GL_ACCUM_BUFFER_BIT);
		glAccum (GL_ACCUM, 1.);
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_motion_blur_mouse_moved (gpointer pUserData, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	CDMotionBlurData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData == NULL)
		pData = g_new0 (CDMotionBlurData, 1);
	
	pData->iBlurCount = 20;
	*bStartAnimation = TRUE;
	
	CD_APPLET_SET_MY_DOCK_DATA (pDock, pData);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_motion_blur_update_dock (gpointer pUserData, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDMotionBlurData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (! pDock->bIsShrinkingDown && ! pDock->bIsGrowingUp)
		pData->iBlurCount --;
	cd_message ("blur <- %d", pData->iBlurCount);
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
	if (pData->iBlurCount <= 0)
	{
		g_free (pData);
		CD_APPLET_SET_MY_DOCK_DATA (pDock, NULL);
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	*bContinueAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
