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
#include "applet-notifications.h"


gboolean cd_illusion_on_remove_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock)
{
	if (! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
	{
		pData = g_new0 (CDIllusionData, 1);
		CD_APPLET_SET_MY_ICON_DATA (pIcon, pData);
	}
	
	gboolean bSartAnimation = FALSE;
	switch (myConfig.iEffect)
	{
		case CD_ILLUSION_EVAPORATE :
			bSartAnimation = cd_illusion_init_evaporate (pIcon, pDock, pData, mySystem.iGLAnimationDeltaT);
		break ;
		case CD_ILLUSION_FADE_OUT :
			bSartAnimation = cd_illusion_init_fade_out (pIcon, pDock, pData, mySystem.iGLAnimationDeltaT);
		break ;
		case CD_ILLUSION_EXPLODE :
			bSartAnimation = cd_illusion_init_explode (pIcon, pDock, pData, mySystem.iGLAnimationDeltaT);
		break ;
	}
	if (bSartAnimation)
		cairo_dock_mark_icon_as_inserting_removing (pIcon);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_illusion_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL || *bHasBeenRendered)
                return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->fEvaporateSpeed != 0)
	{
		cd_illusion_draw_evaporate_icon (pIcon, pDock, pData);
		*bHasBeenRendered = TRUE;
	}
	else if (pData->fFadeOutSpeed != 0)
	{
		cd_illusion_draw_fade_out_icon (pIcon, pDock, pData);
	}
	else if (pData->fExplodeDeltaT != 0)
	{
		cd_illusion_draw_explode_icon (pIcon, pDock, pData);
		*bHasBeenRendered = TRUE;
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_illusion_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->fEvaporateSpeed != 0)
		*bContinueAnimation = cd_illusion_update_evaporate (pIcon, pDock, pData);
	else if (pData->fFadeOutSpeed != 0)
		*bContinueAnimation = cd_illusion_update_fade_out (pIcon, pDock, pData);
	else if (pData->fExplodeDeltaT != 0)
		*bContinueAnimation = cd_illusion_update_explode (pIcon, pDock, pData);
	
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
	
	g_free (pData);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
