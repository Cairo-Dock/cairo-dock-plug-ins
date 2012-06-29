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

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-busy.h"


static void init (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL)
{
	// load the busy animation image once, to avoid loading it for each icon.
	if (myData.pBusyImage == NULL)
		myData.pBusyImage = cairo_dock_create_image_buffer (myConfig.cBusyImage ? myConfig.cBusyImage : MY_APPLET_SHARE_DATA_DIR"/busy.svg",
		0, 0,
		CAIRO_DOCK_ANIMATED_IMAGE);
	
	// copy the image buffer on the icon, because we'll update the frame number for each icon.
	g_free (pData->pBusyImage);
	pData->pBusyImage = g_memdup (myData.pBusyImage, sizeof (CairoDockImageBuffer));
	cairo_dock_image_buffer_set_timelength (pData->pBusyImage, 1.e-3 * myConfig.iBusyDuration);
}

static gboolean update (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bRepeat)
{
	if (! cairo_dock_image_buffer_is_animated (pData->pBusyImage))
		return FALSE;
	
	double fPrevFrame = pData->pBusyImage->iCurrentFrame;
	cairo_dock_image_buffer_next_frame (pData->pBusyImage);
	
	cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (pDock));
	
	return (pData->pBusyImage->iCurrentFrame > fPrevFrame);
}

static void post_render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	g_return_if_fail (pData->pBusyImage);
	
	if (pCairoContext)
	{
		cairo_save (pCairoContext);

		cairo_translate (pCairoContext,
			pIcon->fWidth * pIcon->fScale / 2,
			pIcon->fHeight * pIcon->fScale / 2);
		double z = MIN (pIcon->fScale * pIcon->fWidth / (pData->pBusyImage->iWidth / pData->pBusyImage->iNbFrames), pIcon->fScale * pIcon->fHeight / pData->pBusyImage->iHeight);
		cairo_scale (pCairoContext, z/2, z/2);

		cairo_translate (pCairoContext,
			-pData->pBusyImage->iWidth/pData->pBusyImage->iNbFrames/2,
			-pData->pBusyImage->iHeight/2);

		cairo_dock_apply_image_buffer_surface (pData->pBusyImage, pCairoContext);

		cairo_restore (pCairoContext);
	}
	else
	{
		_cairo_dock_enable_texture ();
		if (pIcon->fAlpha == 1)
			_cairo_dock_set_blend_over ();
		else
			_cairo_dock_set_blend_alpha ();

		double z = MIN (pIcon->fScale * pIcon->fWidth / (pData->pBusyImage->iWidth / pData->pBusyImage->iNbFrames), pIcon->fScale * pIcon->fHeight / pData->pBusyImage->iHeight);
		glScalef (z/2, z/2, 1.);

		cairo_dock_apply_image_buffer_texture (pData->pBusyImage);

		_cairo_dock_disable_texture ();
	}
}


void cd_animations_register_busy (void)
{
	CDAnimation *pAnimation = &myData.pAnimations[CD_ANIMATIONS_BUSY];
	pAnimation->cName = "busy";
	pAnimation->cDisplayedName = D_("Busy");
	pAnimation->id = CD_ANIMATIONS_BUSY;
	pAnimation->bDrawIcon = FALSE;
	pAnimation->bDrawReflect = FALSE;
	pAnimation->init = init;
	pAnimation->update = update;
	pAnimation->render = NULL;
	pAnimation->post_render = post_render;
	cd_animations_register_animation (pAnimation);
}
