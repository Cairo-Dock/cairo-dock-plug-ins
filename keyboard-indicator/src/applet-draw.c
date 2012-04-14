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


/// TODO: remove 'cIndicatorName' if the overlays are a good replacement...
void cd_xkbd_update_icon (const gchar *cGroupName, const gchar *cShortGroupName, const gchar *cIndicatorName, gboolean bRedrawSurface)
{
	//g_print ("%s (%s;%s;%d)\n", __func__, cGroupName, cShortGroupName, bRedrawSurface);
	
	if (bRedrawSurface)  // group has changed -> update the icon and label
	{
		//\__________________ On sauvegarde l'ancienne surface/texture.
		if (myData.pOldSurface != NULL)
			cairo_surface_destroy (myData.pOldSurface);
		if (myData.iOldTexture != 0)
			_cairo_dock_delete_texture (myData.iOldTexture);
		myData.pOldSurface = myData.pCurrentSurface;
		myData.iOldTexture = myData.iCurrentTexture;
		myData.iOldTextWidth = myData.iCurrentTextWidth;
		myData.iOldTextHeight = myData.iCurrentTextHeight;
		
		//\__________________ On cree la nouvelle surface (la taille du texte peut avoir change).
		int iWidth, iHeight;
		CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
		if (iWidth <= 1 && iHeight <= 1)  // peut arriver au lancement en mode desklet.
		{
			myData.pCurrentSurface = NULL;
			myData.iCurrentTexture = 0;
			myData.iCurrentTextWidth = 0;
			myData.iCurrentTextHeight = 0;
			return;
		}
		myData.pCurrentSurface = cairo_dock_create_surface_from_text_full (cShortGroupName,
			&myConfig.textDescription,
			1.,
			0*iWidth,
			&myData.iCurrentTextWidth, &myData.iCurrentTextHeight);
		cd_debug ("KEYBOARD: %dx%d / %dx%d", myData.iCurrentTextWidth, myData.iCurrentTextHeight, myIcon->iImageWidth, myIcon->iImageHeight);
		if (g_bUseOpenGL)
		{
			myData.iCurrentTexture = cairo_dock_create_texture_from_surface (myData.pCurrentSurface);
		}
		
		//\__________________ On lance une transition entre ancienne et nouvelle surface/texture, ou on dessine direct.
		if (myConfig.iTransitionDuration != 0 && myData.pOldSurface != NULL)
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
	else  // only the indicators have changed -> trigger a redraw event only  (overlay update).
	{
		CD_APPLET_REDRAW_MY_ICON;
	}
	
	//\__________________ lock indicators
	///CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (!cIndicatorName || *cIndicatorName == '\0' ? NULL : cIndicatorName);
	if (myConfig.bShowKbdIndicator)
	{
		cd_debug ("XKBD: caps-lock: %d; num-lock: %d", myData.iCurrentIndic & 1, myData.iCurrentIndic & 2);
		if (myData.iCurrentIndic & 1)  // caps-lock
		{
			if (! (myData.iPreviousIndic & 1))
				CD_APPLET_ADD_OVERLAY_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/caps-lock.png", CAIRO_OVERLAY_UPPER_RIGHT);
		}
		else
		{
			if (myData.iPreviousIndic & 1)
				CD_APPLET_REMOVE_OVERLAY_ON_MY_ICON (CAIRO_OVERLAY_UPPER_RIGHT);
		}

		if (myData.iCurrentIndic & 2)  // num-lock
		{
			if (! (myData.iPreviousIndic & 2))
				CD_APPLET_ADD_OVERLAY_ON_MY_ICON (MY_APPLET_SHARE_DATA_DIR"/num-lock.png", CAIRO_OVERLAY_UPPER_LEFT);
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
	if (fTheta < 25 && myData.iOldTexture != 0)  // inutile de dessiner si elle est derriere l'image courante, par l'effet de perspective (en fait 22.5, mais bizarrement ca a l'air un peu trop tot).
	{
		w = iWidth * myConfig.fTextRatio;  // fill horizontally
		h = myData.iOldTextHeight * (double)w/myData.iOldTextWidth;  // keep ratio
		if (h > iHeight * myConfig.fTextRatio)
		{
			w *= iHeight * myConfig.fTextRatio / h;
			h = iHeight * myConfig.fTextRatio;
		}
		
		glPushMatrix ();
		glRotatef (45. + fTheta, 0., 1., 0.);  // 0 -> 90
		glTranslatef (0., (-iHeight + h)/2, w/2);  // H center, V bottom
		cairo_dock_apply_texture_at_size (myData.iOldTexture, w, h);
		glPopMatrix ();
	}
	
	// image courante a 90deg.
	w = iWidth * myConfig.fTextRatio;  // fill horizontally
	h = myData.iCurrentTextHeight * (double)w/myData.iCurrentTextWidth;  // keep ratio
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
	cairo_dock_apply_texture_at_size (myData.iCurrentTexture, w, h);
	
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
	
	cairo_dock_erase_cairo_context (myDrawContext);
	
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
	if (myData.pOldSurface != NULL && 1-f > .01)
	{
		fScale = (double)iWidth / myData.iOldTextWidth;  // scale to fill the icon horizontally
		if (fScale * myData.iOldTextHeight > iHeight)  // if the text is too height, scale down
		{
			fScale = (double)iHeight / myData.iOldTextHeight;  // that's smaller than the previous value.
		}
		dx = (iWidth - fScale * myData.iOldTextWidth)/2;  // center horizontally
		dy = iHeight - fScale * myData.iOldTextHeight;  // bottom (we draw the caps/num lock on top).
		
		cairo_save (myDrawContext);
		cairo_translate (myDrawContext, dx, dy);
		cairo_scale (myDrawContext, fScale, fScale);  // keep ratio
		cairo_set_source_surface (
			myDrawContext,
			myData.pOldSurface,
			0,
			0);
		cairo_paint_with_alpha (myDrawContext, 1-f);
		cairo_restore (myDrawContext);
	}
	
	if (myData.pCurrentSurface != NULL)
	{
		fScale = (double)iWidth / myData.iCurrentTextWidth;  // scale to fill the icon horizontally
		if (fScale * myData.iCurrentTextHeight > iHeight)  // if the text is too height, scale down
		{
			fScale = (double)iHeight / myData.iCurrentTextHeight;  // that's smaller than the previous value.
		}
		dx = (iWidth - fScale * myData.iCurrentTextWidth)/2;  // center horizontally
		dy = iHeight - fScale * myData.iCurrentTextHeight;  // bottom (we draw the caps/num lock on top).
		
		cairo_save (myDrawContext);
		cairo_translate (myDrawContext, dx, dy);
		cairo_scale (myDrawContext, fScale, fScale);  // keep ratio
		cairo_set_source_surface (
			myDrawContext,
			myData.pCurrentSurface,
			0,
			0);
		cairo_paint_with_alpha (myDrawContext, f);
		cairo_restore (myDrawContext);
	}
	
	CD_APPLET_LEAVE (TRUE);
}
