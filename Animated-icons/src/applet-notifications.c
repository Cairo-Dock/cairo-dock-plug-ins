/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-icon-renderer.h"
#include "applet-spot.h"
#include "applet-struct.h"
#include "applet-rays.h"
#include "applet-wobbly.h"
#include "applet-notifications.h"


gboolean cd_animations_start (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (pIcon->bStatic || ! CAIRO_DOCK_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		pData = g_new0 (CDAnimationData, 1);
	
	double dt = g_iGLAnimationDeltaT;
	
	if (myConfig.iRotationDuration != 0)
	{
		if (myData.iChromeTexture == 0)
			myData.iChromeTexture = cd_animation_load_chrome_texture ();
		pData->fRotationSpeed = 360. / myConfig.iRotationDuration * dt;
		pData->fRotationBrake = 1.;
		pData->fAdjustFactor = 0.;
		pData->bRotationBeginning = TRUE;
		*bStartAnimation = TRUE;
	}
	
	if (myConfig.iSpotDuration != 0)
	{
		if (myData.iSpotTexture == 0)
			myData.iSpotTexture = cd_animation_load_spot_texture ();
		if (myData.iHaloTexture == 0)
			myData.iHaloTexture = cd_animation_load_halo_texture ();
		if (myData.iSpotFrontTexture == 0)
			myData.iSpotFrontTexture = cd_animation_load_spot_front_texture ();
		if (myData.iRaysTexture == 0)
			myData.iRaysTexture = cd_animations_load_rays_texture ();
		if (pData->pRaysSystem == NULL)
			pData->pRaysSystem = cd_animations_init_rays (pIcon, pDock, dt);
		pData->fRadiusFactor = .001;
		pData->fHaloRotationAngle = 0;
		*bStartAnimation = TRUE;
	}
	
	if (myConfig.fSpringConstant != 0)
	{
		cd_animations_init_wobbly (pData);
		*bStartAnimation = TRUE;
	}
	
	
	CD_APPLET_SET_MY_ICON_DATA (pIcon, pData);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


static void _cd_animations_render_rays (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, int iDepth)
{
	glPushMatrix ();
	if (pDock->bHorizontalDock)
		glTranslatef (0., - pIcon->fHeight * pIcon->fScale/2, 0.);
	else
		glTranslatef (- pIcon->fHeight * pIcon->fScale/2, 0., 0.);
	
	if (! pDock->bHorizontalDock)
		glRotatef (-90, 0., 0., 1.);
	
	if (pData->pRaysSystem != NULL)
	{
		cairo_dock_render_particles_full (pData->pRaysSystem, iDepth);
	}

	glPopMatrix ();
}
gboolean cd_animations_post_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->fRadiusFactor != 0)
	{
		if (pDock->bHorizontalDock)
			glTranslatef (0., - pData->fIconOffsetY * (pDock->bDirectionUp ? 1 : -1), 0.);
		else
			glTranslatef (- pData->fIconOffsetY * (pDock->bDirectionUp ? -1 : 1), 0., 0.);
		if (pData->pRaysSystem != NULL)
			_cd_animations_render_rays (pIcon, pDock, pData, 1);
		
		cd_animation_render_spot_front (pIcon, pDock, pData->fRadiusFactor);
		if (pData->fHaloRotationAngle > 90 && pData->fHaloRotationAngle < 270)
			cd_animation_render_halo (pIcon, pDock, pData->fRadiusFactor, pData->fHaloRotationAngle);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean cd_animations_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered)
{
	if (*bHasBeenRendered)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->fRadiusFactor != 0)
	{
		cd_animation_render_spot (pIcon, pDock, pData->fRadiusFactor);
		if (pData->fHaloRotationAngle <= 90 || pData->fHaloRotationAngle >= 270)
			cd_animation_render_halo (pIcon, pDock, pData->fRadiusFactor, pData->fHaloRotationAngle);
		
		if (pData->pRaysSystem != NULL)
			_cd_animations_render_rays (pIcon, pDock, pData, 1);
		
		if (pDock->bHorizontalDock)
			glTranslatef (0., pData->fIconOffsetY * (pDock->bDirectionUp ? 1 : -1), 0.);
		else
			glTranslatef (pData->fIconOffsetY * (pDock->bDirectionUp ? -1 : 1), 0., 0.);
	}
	
	if (pData->bIsWobblying)
	{
		cd_animations_draw_wobbly_icon (pIcon, pDock, pData);
		*bHasBeenRendered = TRUE;
	}
	else if (pData->fRotationSpeed != 0)
	{
		gboolean bInvisibleBackground = (pDock->bInside);
		bInvisibleBackground = TRUE;
		glPushMatrix ();
		if (pDock->bHorizontalDock)
			glTranslatef (0., pData->fIconOffsetY * (pDock->bDirectionUp ? 1 : -1), 0.);
		else
			glTranslatef (pData->fIconOffsetY * (pDock->bDirectionUp ? -1 : 1), 0., 0.);
		glRotatef (pData->fRotationAngle, 0., 1., 0.);
		switch (myConfig.iMeshType)
		{
			case CD_SQUARE_MESH :
			default :
				cairo_dock_set_icon_scale (pIcon, pDock, 1.);
				cd_animation_render_square (pIcon, pDock, bInvisibleBackground);
			break;
			case CD_CUBE_MESH :
				glRotatef (fabs (pData->fRotationAngle/4), 1., 0., 0.);
				cairo_dock_set_icon_scale (pIcon, pDock, 1. + pData->fAdjustFactor * (sqrt (2) - 1));
				cd_animation_render_cube (pIcon, pDock, bInvisibleBackground);
			break;
			case CD_CAPSULE_MESH :
				cairo_dock_set_icon_scale (pIcon, pDock, 1.);
				cd_animation_render_capsule (pIcon, pDock, bInvisibleBackground);
			break;
		}
		glPopMatrix ();
		*bHasBeenRendered = TRUE;
	}
	
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_animations_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pData->bIsWobblying)
	{
		pData->bIsWobblying = cd_animations_update_wobbly (pData);
		if (pData->bIsWobblying)
			*bContinueAnimation = TRUE;
	}
	if (! pData->bIsWobblying && pData->fRotationSpeed != 0)
	{
		double delta;
		if (pData->fRotationAngle < 40)
		{
			if (pData->bRotationBeginning)
				pData->fAdjustFactor = (40 - pData->fRotationAngle) / (40. - 0.);
		}
		else if (pData->bRotationBeginning)
			pData->bRotationBeginning = FALSE;
		if (pData->fRotationAngle > 320)
		{
			if (! myConfig.bContinueRotation || ! pIcon->bPointed || ! pDock->bInside)
			{
				pData->fRotationBrake = MAX (.2, (360. - pData->fRotationAngle) / (360. - 320.));
				pData->fAdjustFactor = (pData->fRotationAngle - 320) / (360. - 320.);
			}
		}
		pData->fRotationAngle += pData->fRotationSpeed * pData->fRotationBrake;
		if (pData->fRotationAngle < 360)
			*bContinueAnimation = TRUE;
		else
		{
			pData->fRotationAngle = 0;
			if (myConfig.bContinueRotation && pIcon->bPointed && pDock->bInside)
			{
				*bContinueAnimation = TRUE;
			}
			else
			{
				pData->fRotationSpeed = 0;
			}
		}
	}
	
	if (pData->fRadiusFactor != 0)
	{
		if (pIcon->bPointed && pDock->bInside)
		{
			pData->fRadiusFactor += 1./myConfig.iSpotDuration * g_iGLAnimationDeltaT;
			if (pData->fRadiusFactor > 1)
				pData->fRadiusFactor = 1.;
			else
				*bContinueAnimation = TRUE;
			pData->fIconOffsetY += 1.*myLabels.iconTextDescription.iSize / myConfig.iSpotDuration * g_iGLAnimationDeltaT;
			if (pData->fIconOffsetY > myLabels.iconTextDescription.iSize)
				pData->fIconOffsetY = myLabels.iconTextDescription.iSize;
			else
				*bContinueAnimation = TRUE;
		}
		else
		{
			pData->fRadiusFactor -= 1./myConfig.iSpotDuration * g_iGLAnimationDeltaT;
			if (pData->fRadiusFactor < 0)
				pData->fRadiusFactor = 0.;
			else
				*bContinueAnimation = TRUE;
			pData->fIconOffsetY -= 1.*myLabels.iconTextDescription.iSize / myConfig.iSpotDuration * g_iGLAnimationDeltaT;
			if (pData->fIconOffsetY < 0)
				pData->fIconOffsetY = 0.;
			else
				*bContinueAnimation = TRUE;
		}
		pIcon->fDeltaYReflection += 2 * pData->fIconOffsetY;
		
		pData->fHaloRotationAngle += 360. / myConfig.iSpotDuration * g_iGLAnimationDeltaT;
		if (pData->fHaloRotationAngle < 360)
		{
			*bContinueAnimation = TRUE;
		}
		else
		{
			pData->fHaloRotationAngle = 0;
			if (myConfig.bContinueSpot && pIcon->bPointed && pDock->bInside)
			{
				*bContinueAnimation = TRUE;
			}
		}
		
		if (pData->pRaysSystem != NULL)
		{
			gboolean bContinueSpot = cd_animations_update_rays_system (pData->pRaysSystem,
			(myConfig.bContinueSpot && pIcon->bPointed && pDock->bInside));
			pData->pRaysSystem->fWidth = pIcon->fWidth * pIcon->fScale * pData->fRadiusFactor;
			if (bContinueSpot)
				*bContinueAnimation = TRUE;
			else
			{
				cairo_dock_free_particle_system (pData->pRaysSystem);
				pData->pRaysSystem = NULL;
			}
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_animations_free_data (gpointer pUserData, Icon *pIcon)
{
	g_print ("%s ()\n", __func__);
	CDAnimationData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_free (pData);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
