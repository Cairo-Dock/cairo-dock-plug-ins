/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"

#define CD_DOCK_IN_MOVMENT(pDock) ((myConfig.bAlways && pDock->iSidGLAnimation) || pDock->bIsShrinkingDown || pDock->bIsGrowingUp)


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
	//g_print ("blur <- %d\n", pData->iBlurCount);
	
	if (pData->iBlurCount <= 0)
	{
		g_free (pData);
		CD_APPLET_SET_MY_DOCK_DATA (pDock, NULL);
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	*bContinueAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
