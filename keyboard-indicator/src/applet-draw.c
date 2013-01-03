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
#include <math.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-draw.h"


void cd_xkbd_update_icon (const gchar *cGroupName, const gchar *cShortGroupName, gboolean bRedrawSurface)
{
	//g_print ("%s (%s;%s;%d)\n", __func__, cGroupName, cShortGroupName, bRedrawSurface);
	
	if (bRedrawSurface)  // group has changed -> update the icon and label
	{
		//\__________________ On sauvegarde l'ancienne surface/texture.
		cairo_dock_free_image_buffer (myData.pOldImage);
		myData.pOldImage = myData.pCurrentImage;
		myData.pCurrentImage = NULL;
		/**if (myData.pOldSurface != NULL)
			cairo_surface_destroy (myData.pOldSurface);
		if (myData.pOldImage->iure != 0)
			_cairo_dock_delete_texture (myData.pOldImage->iure);
		myData.pOldSurface = myData.pCurrentSurface;
		myData.pOldImage->iure = myData.pCurrentImage->iure;
		myData.pOldImage->iWidth = myData.pCurrentImage->iWidth;
		myData.pOldImage->iHeight = myData.pCurrentImage->iHeight;*/
		
		//\__________________ On cree la nouvelle surface (la taille du texte peut avoir change).
		int iWidth, iHeight;
		CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
		if (iWidth <= 1 && iHeight <= 1)  // peut arriver au lancement en mode desklet.
		{
			/**myData.pCurrentSurface = NULL;
			myData.pCurrentImage->iure = 0;
			myData.pCurrentImage->iWidth = 0;
			myData.pCurrentImage->iHeight = 0;*/
			return;
		}
		int w, h;
		cairo_surface_t *pSurface = cairo_dock_create_surface_from_text_full (cShortGroupName,
			&myConfig.textDescription,
			1.,
			0,  /// iWidth
			&w, &h);
		myData.pCurrentImage = g_new0 (CairoDockImageBuffer, 1);
		cairo_dock_load_image_buffer_from_surface (myData.pCurrentImage, pSurface, w, h);
		/**myData.pCurrentSurface = cairo_dock_create_surface_from_text_full (cShortGroupName,
			&myConfig.textDescription,
			1.,
			0*iWidth,
			&myData.pCurrentImage->iWidth, &myData.pCurrentImage->iHeight);
		cd_debug ("KEYBOARD: %dx%d / %dx%d", myData.pCurrentImage->iWidth, myData.pCurrentImage->iHeight, myIcon->image.iWidth, myIcon->image.iHeight);
		if (g_bUseOpenGL)
		{
			myData.pCurrentImage->iure = cairo_dock_create_texture_from_surface (myData.pCurrentSurface);
		}*/
		
		//\__________________ On lance une transition entre ancienne et nouvelle surface/texture, ou on dessine direct.
		if (myConfig.iTransitionDuration != 0 && myData.pOldImage != NULL)
		{
			CD_APPLET_SET_TRANSITION_ON_MY_ICON (cd_xkbd_render_step_cairo,
				cd_xkbd_render_step_opengl,
				g_bUseOpenGL,  // bFastPace : vite si opengl, lent si cairo.
				myConfig.iTransitionDuration,
				TRUE);  // bRemoveWhenFinished
		}
		else
		{
			if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
			{
				CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();
				cd_xkbd_render_step_opengl (myIcon, myApplet);	
				CD_APPLET_FINISH_DRAWING_MY_ICON;
			}
			else
			{
				cd_xkbd_render_step_cairo (myIcon, myApplet);
				///CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
			}
			CD_APPLET_REDRAW_MY_ICON;
		}
		
		//\__________________ update the label.
		CD_APPLET_SET_NAME_FOR_MY_ICON (cGroupName);
	}
	else  // only the indicators have changed -> trigger a redraw event only (overlay update).
	{
		CD_APPLET_REDRAW_MY_ICON;
	}
	
	//\__________________ lock indicators
	if (myConfig.bShowKbdIndicator)
	{
		cd_debug ("XKBD: caps-lock: %d; num-lock: %d", myData.iCurrentIndic & 1, myData.iCurrentIndic & 2);
		if (myData.iCurrentIndic & 1)  // caps-lock
		{
			if (! (myData.iPreviousIndic & 1)) // TODO: cairo_dock_search_icon_s_path in init? or here the first time (save to data) and reset in reload?
			{
				if (myConfig.cEmblemCapsLock && (myData.cEmblemCapsLock ||
					(myData.cEmblemCapsLock = cairo_dock_search_icon_s_path (myConfig.cEmblemCapsLock, // search for an icon only the first time
						MAX (myIcon->image.iWidth/2, myIcon->image.iHeight/2)))))
					CD_APPLET_ADD_OVERLAY_ON_MY_ICON (myData.cEmblemCapsLock, CAIRO_OVERLAY_UPPER_RIGHT);
				else
					CD_APPLET_ADD_OVERLAY_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/caps-lock.png", CAIRO_OVERLAY_UPPER_RIGHT);
			}
		}
		else
		{
			if (myData.iPreviousIndic & 1)
				CD_APPLET_REMOVE_OVERLAY_ON_MY_ICON (CAIRO_OVERLAY_UPPER_RIGHT);
		}

		if (myData.iCurrentIndic & 2)  // num-lock
		{
			if (! (myData.iPreviousIndic & 2))
			{
				if (myConfig.cEmblemNumLock &&(myData.cEmblemNumLock ||
					(myData.cEmblemNumLock = cairo_dock_search_icon_s_path (myConfig.cEmblemNumLock,
						MAX (myIcon->image.iWidth/2, myIcon->image.iHeight/2)))))
					CD_APPLET_ADD_OVERLAY_ON_MY_ICON (myData.cEmblemNumLock, CAIRO_OVERLAY_UPPER_LEFT);
				else
					CD_APPLET_ADD_OVERLAY_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/num-lock.png", CAIRO_OVERLAY_UPPER_LEFT);
			}
		}
		else
		{
			if (myData.iPreviousIndic & 2)
				CD_APPLET_REMOVE_OVERLAY_ON_MY_ICON (CAIRO_OVERLAY_UPPER_LEFT);
		}
		myData.iPreviousIndic = myData.iCurrentIndic;
	}
}


gboolean cd_xkbd_render_step_opengl (Icon *pIcon, CairoDockModuleInstance *myApplet)
{
	g_return_val_if_fail (myData.pCurrentImage != NULL, FALSE);
	CD_APPLET_ENTER;
	double f = CD_APPLET_GET_TRANSITION_FRACTION ();
	cd_debug ("%s (%.2f; %.2fx%.2f)", __func__, f, myIcon->fWidth, myIcon->fHeight);
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	cairo_dock_set_perspective_view_for_icon (myIcon, myContainer);
	glScalef (1., -1., 1.);
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	// fond
	if (myData.bgImage.iTexture != 0)
		cairo_dock_apply_texture_at_size (myData.bgImage.iTexture, iWidth, iHeight);
	
	double fTheta = - 45. + f * 90.;  // -45 -> 45
	glTranslatef (0., 0., - iWidth * sqrt(2)/2 * cos (fTheta/180.*G_PI));  // pour faire tenir le cube dans la fenetre.
	glEnable (GL_DEPTH_TEST);
	
	// image precedente.
	int w=0, h;
	if (fTheta < 25 && myData.pOldImage != NULL)  // inutile de dessiner si elle est derriere l'image courante, par l'effet de perspective (en fait 22.5, mais bizarrement ca a l'air un peu trop tot).
	{
		w = iWidth * myConfig.fTextRatio;  // fill horizontally
		h = myData.pOldImage->iHeight * (double)w/myData.pOldImage->iWidth;  // keep ratio
		if (h > iHeight * myConfig.fTextRatio)
		{
			w *= iHeight * myConfig.fTextRatio / h;
			h = iHeight * myConfig.fTextRatio;
		}
		
		glPushMatrix ();
		glRotatef (45. + fTheta, 0., 1., 0.);  // 0 -> 90
		glTranslatef (0., (-iHeight + h)/2, w/2);  // H center, V bottom
		cairo_dock_apply_texture_at_size (myData.pOldImage->iTexture, w, h);
		glPopMatrix ();
	}
	
	// image courante a 90deg.
	w = iWidth * myConfig.fTextRatio;  // fill horizontally
	h = myData.pCurrentImage->iHeight * (double)w/myData.pCurrentImage->iWidth;  // keep ratio
	if (h > iHeight * myConfig.fTextRatio)
	{
		w *= iHeight * myConfig.fTextRatio / h;
		h = iHeight * myConfig.fTextRatio;
	}

	/**glRotatef (45. + fTheta, 0., 1., 0.);  // 0 -> 90
	glTranslatef (- (w ? w : iWidth)/2, 0., 0.);
	glRotatef (-90., 0., 1., 0.);*/
	glRotatef (-45. + fTheta, 0., 1., 0.);  // -90 -> 0
	glTranslatef (0., (-iHeight + h)/2, w/2);  // H center, V bottom
	cairo_dock_apply_texture_at_size (myData.pCurrentImage->iTexture, w, h);
	
	glDisable (GL_DEPTH_TEST);
	_cairo_dock_disable_texture ();
	
	if (myDock)
	{
		cairo_dock_set_ortho_view (myContainer);
	}
	
	CD_APPLET_LEAVE (TRUE);
}


gboolean cd_xkbd_render_step_cairo (Icon *pIcon, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	double f = CD_APPLET_GET_TRANSITION_FRACTION ();
	
	//g_print ("%s (%.2f)\n", __func__, f);
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	CD_APPLET_LEAVE_IF_FAIL (iHeight != 0, TRUE);
	
	///cairo_dock_erase_cairo_context (myDrawContext);
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO (FALSE);

	if (myData.bgImage.pSurface != NULL)
	{
		cairo_set_source_surface (
			myDrawContext,
			myData.bgImage.pSurface,
			0.,
			0.);
		cairo_paint (myDrawContext);
	}
	
	double dx, dy, fScale;
	if (myData.pOldImage != NULL && 1-f > .01)
	{
		fScale = (double)iWidth / myData.pOldImage->iWidth;  // scale to fill the icon horizontally
		if (fScale * myData.pOldImage->iHeight > iHeight)  // if the text is too height, scale down
		{
			fScale = (double)iHeight / myData.pOldImage->iHeight;  // that's smaller than the previous value.
		}
		dx = (iWidth - fScale * myData.pOldImage->iWidth)/2;  // center horizontally
		dy = iHeight - fScale * myData.pOldImage->iHeight;  // bottom (we draw the caps/num lock on top).
		
		cairo_save (myDrawContext);
		cairo_translate (myDrawContext, dx, dy);
		cairo_scale (myDrawContext, fScale, fScale);  // keep ratio
		cairo_set_source_surface (
			myDrawContext,
			myData.pOldImage->pSurface,
			0,
			0);
		cairo_paint_with_alpha (myDrawContext, 1-f);
		cairo_restore (myDrawContext);
	}
	
	if (myData.pCurrentImage != NULL)
	{
		fScale = (double)iWidth / myData.pCurrentImage->iWidth;  // scale to fill the icon horizontally
		if (fScale * myData.pCurrentImage->iHeight > iHeight)  // if the text is too height, scale down
		{
			fScale = (double)iHeight / myData.pCurrentImage->iHeight;  // that's smaller than the previous value.
		}
		dx = (iWidth - fScale * myData.pCurrentImage->iWidth)/2;  // center horizontally
		dy = iHeight - fScale * myData.pCurrentImage->iHeight;  // bottom (we draw the caps/num lock on top).
		
		cairo_save (myDrawContext);
		cairo_translate (myDrawContext, dx, dy);
		cairo_scale (myDrawContext, fScale, fScale);  // keep ratio
		cairo_set_source_surface (
			myDrawContext,
			myData.pCurrentImage->pSurface,
			0,
			0);
		cairo_paint_with_alpha (myDrawContext, f);
		cairo_restore (myDrawContext);
	}
	
	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
	CD_APPLET_LEAVE (TRUE);
}
