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

#include "applet-struct.h"
#include "applet-notifications.h"

#define CD_DOCK_IN_MOVMENT(pDock) ((myConfig.bAlways && cairo_dock_container_is_animating (CAIRO_CONTAINER(pDock))) || pDock->bIsShrinkingDown || pDock->bIsGrowingUp)


gboolean cd_motion_blur_pre_render (gpointer pUserData, CairoDock *pDock, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	CDMotionBlurData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	
	if ((pData != NULL && pData->iBlurCount != 0) || CD_DOCK_IN_MOVMENT (pDock))
		glAccum(GL_MULT, myConfig.fBlurFactor);
	
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_motion_blur_post_render (gpointer pUserData, CairoDock *pDock, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL)
		return GLDI_NOTIFICATION_LET_PASS;
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
	
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_motion_blur_mouse_moved (gpointer pUserData, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))
		return GLDI_NOTIFICATION_LET_PASS;
	CDMotionBlurData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData == NULL)
	{
		pData = g_new0 (CDMotionBlurData, 1);
		CD_APPLET_SET_MY_DOCK_DATA (pDock, pData);
	}
	pData->iBlurCount = -3 / log (myConfig.fBlurFactor);  // blur applied N times to get 10^-3
	*bStartAnimation = TRUE;
	
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_motion_blur_update_dock (gpointer pUserData, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDMotionBlurData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if (! pDock->bIsShrinkingDown && ! pDock->bIsGrowingUp)
		pData->iBlurCount --;
	//g_print ("blur <- %d\n", pData->iBlurCount);
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
	if (pData->iBlurCount <= 0)
	{
		g_free (pData);
		CD_APPLET_SET_MY_DOCK_DATA (pDock, NULL);
		return GLDI_NOTIFICATION_LET_PASS;
	}
	
	*bContinueAnimation = TRUE;
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_motion_free_data (gpointer pUserData, CairoDock *pDock)
{
	CDMotionBlurData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData != NULL)
	{
		g_free (pData);
		CD_APPLET_SET_MY_DOCK_DATA (pDock, NULL);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}