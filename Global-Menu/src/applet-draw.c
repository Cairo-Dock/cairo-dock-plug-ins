/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* based on indicator-messages.c written by :
*  Ted Gould <ted@canonical.com>
*  Cody Russell <cody.russell@canonical.com>
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
#include "applet-draw.h"

#define _compute_alpha(pImage, iAnimIter, bEnabled) (bEnabled ? \
		cairo_dock_image_buffer_is_animated (pImage) ? \
			1. : \
			1. - .4 * sin (G_PI * iAnimIter / (CD_ANIM_STEPS - 1))\
		: .6)
static void _apply_button_opengl (CairoDockImageBuffer *pImage, int x, int y, gboolean bEnabled, gint iAnimIter)
{
	double fAlpha = _compute_alpha (pImage, iAnimIter, bEnabled);
	_cairo_dock_set_alpha (fAlpha);
	cairo_dock_apply_image_buffer_texture_with_offset (pImage, x, y);
}

static void _apply_button_cairo (CairoDockImageBuffer *pImage, int x, int y, gboolean bEnabled, gint iAnimIter)
{
	double fAlpha = _compute_alpha (pImage, iAnimIter, bEnabled);
	cairo_dock_apply_image_buffer_surface_with_offset (pImage, myDrawContext,
		x, y,
		fAlpha);
}

static gboolean cd_app_menu_render_step_opengl (Icon *pIcon, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	double f = CD_APPLET_GET_TRANSITION_FRACTION ();
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	CD_APPLET_LEAVE_IF_FAIL (iHeight != 0, TRUE);
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	
	// icon position/size
	int x, y, w, h;
	if (myConfig.bDisplayControls)
	{
		w = MIN (iWidth, iHeight);
		h = w;
	}
	else
	{
		w = iWidth;
		h = iHeight;
	}
	if (iWidth > iHeight)  // horizontal alignment
	{
		x = (-iWidth + w)/2;  // on the left
		y = 0;  // vertically centered
	}
	else  // vertical alignment
	{
		x = 0;  // horizontally centered
		y = (iHeight - h)/2;  // on top
	}
	
	// draw current icon
	const CairoDockImageBuffer *pImage = NULL, *pPrevImage = NULL;
	
	Icon *pAppli = cairo_dock_get_appli_icon (myData.pCurrentWindow);
	if (pAppli)
	{
		pImage = gldi_appli_icon_get_image_buffer (pAppli);
	}
	GLuint iTexture = (pImage ? pImage->iTexture : 0);
	
	Icon *pPrevIcon = cairo_dock_get_appli_icon (myData.pPreviousWindow);
	if (pPrevIcon)
	{
		pPrevImage = gldi_appli_icon_get_image_buffer (pPrevIcon);
	}
	GLuint iPrevTexture = (pPrevImage ? pPrevImage->iTexture : 0);
	
	if (iPrevTexture != 0)
	{
		_cairo_dock_set_alpha (1-f);
		glBindTexture (GL_TEXTURE_2D, iPrevTexture);
		_cairo_dock_apply_current_texture_at_size_with_offset (w, h, x, y);
	}
	if (iTexture != 0)
	{
		_cairo_dock_set_alpha (f);
		glBindTexture (GL_TEXTURE_2D, iTexture);
		_cairo_dock_apply_current_texture_at_size_with_offset (w, h, x, y);
	}
	_cairo_dock_set_alpha (1);
	
	// draw window buttons
	if (myConfig.bDisplayControls)
	{
		// minimize button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y -= w;
		
		if (myData.bReversedButtonsOrder)
			_apply_button_opengl (&myData.closeButton, x, y, myData.pCurrentWindow && myData.bCanClose, myData.iAnimIterClose);
		else
			_apply_button_opengl (&myData.minimizeButton, x, y, myData.pCurrentWindow && myData.bCanMinimize, myData.iAnimIterMin);

		// restore/maximize button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y -= w;

		if (myData.bReversedButtonsOrder)
			_apply_button_opengl (&myData.minimizeButton, x, y, myData.pCurrentWindow && myData.bCanMinimize, myData.iAnimIterMin);
		else
		{
			if (myData.pCurrentWindow && myData.pCurrentWindow->bIsMaximized)
				_apply_button_opengl (&myData.restoreButton, x, y, myData.pCurrentWindow && myData.bCanMaximize, myData.iAnimIterRestore);
			else
				_apply_button_opengl (&myData.maximizeButton, x, y, myData.pCurrentWindow && myData.bCanMaximize, myData.iAnimIterMax);
		}
		
		// close button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y -= h;

		if (myData.bReversedButtonsOrder)
		{
			if (myData.pCurrentWindow && myData.pCurrentWindow->bIsMaximized)
				_apply_button_opengl (&myData.restoreButton, x, y, myData.pCurrentWindow && myData.bCanMaximize, myData.iAnimIterRestore);
			else
				_apply_button_opengl (&myData.maximizeButton, x, y, myData.pCurrentWindow && myData.bCanMaximize, myData.iAnimIterMax);
		}
		else
			_apply_button_opengl (&myData.closeButton, x, y, myData.pCurrentWindow && myData.bCanClose, myData.iAnimIterClose);
	}
	_cairo_dock_disable_texture ();
	
	CD_APPLET_LEAVE (TRUE);
}

static gboolean cd_app_menu_render_step_cairo (Icon *pIcon, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	double f = CD_APPLET_GET_TRANSITION_FRACTION ();
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	CD_APPLET_LEAVE_IF_FAIL (iHeight != 0, TRUE);
	
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO (FALSE);
	
	// items size
	int x, y, w, h;
	if (myConfig.bDisplayControls)  // we need to draw the icon + 3 buttons
	{
		if (iWidth > iHeight)  // horizontal alignment
		{
			w = MIN (iWidth / 4, iHeight);
			h = w;
		}
		else
		{
			w = MIN (iHeight / 4, iWidth);
			h = w;
		}
	}
	else  // just draw the icon on the whole surface.
	{
		w = iWidth;
		h = iHeight;
	}
	
	// draw current icon
	if (iWidth > iHeight)  // horizontal alignment
	{
		x = 0;  // on the left
		y = (-iHeight + h)/2;  // vertically centered
	}
	else  // vertical alignment
	{
		x = (iWidth - w)/2;  // horizontally centered
		y = 0;  // on top
	}
	
	const CairoDockImageBuffer *pImage = NULL, *pPrevImage = NULL;
	
	Icon *pPrevIcon = cairo_dock_get_appli_icon (myData.pPreviousWindow);
	if (pPrevIcon)
	{
		pPrevImage = gldi_appli_icon_get_image_buffer (pPrevIcon);
	}
	if (pPrevImage && pPrevImage->pSurface)
	{
		cairo_dock_apply_image_buffer_surface_at_size (pPrevImage, myDrawContext, w, h, x, y, 1-f);
	}
	
	Icon *pAppli = cairo_dock_get_appli_icon (myData.pCurrentWindow);
	if (pAppli)
	{
		pImage = gldi_appli_icon_get_image_buffer (pAppli);
	}
	if (pImage && pImage->pSurface)
	{
		cairo_dock_apply_image_buffer_surface_at_size (pImage, myDrawContext, w, h, x, y, f);
	}
	
	// draw window buttons
	if (myConfig.bDisplayControls)
	{
		// minimize button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y += h;
		
		if (myData.bReversedButtonsOrder)
			_apply_button_cairo (&myData.closeButton, x, y, myData.pCurrentWindow && myData.bCanClose, myData.iAnimIterClose);
		else
			_apply_button_cairo (&myData.minimizeButton, x, y, myData.pCurrentWindow && myData.bCanMinimize, myData.iAnimIterMin);
		
		// restore/maximize button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y += h;

		if (myData.bReversedButtonsOrder)
			_apply_button_cairo (&myData.minimizeButton, x, y, myData.pCurrentWindow && myData.bCanMinimize, myData.iAnimIterMin);
		else
		{
			if (myData.pCurrentWindow && myData.pCurrentWindow->bIsMaximized)
				_apply_button_cairo (&myData.maximizeButton, x, y, myData.pCurrentWindow && myData.bCanMaximize, myData.iAnimIterMax);
			else
				_apply_button_cairo (&myData.restoreButton, x, y, myData.pCurrentWindow && myData.bCanMaximize, myData.iAnimIterRestore);
		}
		
		// close button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y += h;

		if (myData.bReversedButtonsOrder)
		{
			if (myData.pCurrentWindow && myData.pCurrentWindow->bIsMaximized)
				_apply_button_cairo (&myData.maximizeButton, x, y, myData.pCurrentWindow && myData.bCanMaximize, myData.iAnimIterMax);
			else
				_apply_button_cairo (&myData.restoreButton, x, y, myData.pCurrentWindow && myData.bCanMaximize, myData.iAnimIterRestore);
		}
		else
			_apply_button_cairo (&myData.closeButton, x, y, myData.pCurrentWindow && myData.bCanClose, myData.iAnimIterClose);
	}
	
	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
	CD_APPLET_LEAVE (TRUE);
}


void cd_app_menu_load_button_images (void)
{
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	g_return_if_fail (iHeight != 0);
	
	cairo_dock_unload_image_buffer (&myData.minimizeButton);
	cairo_dock_unload_image_buffer (&myData.maximizeButton);
	cairo_dock_unload_image_buffer (&myData.restoreButton);
	cairo_dock_unload_image_buffer (&myData.closeButton);
	
	if (myConfig.bDisplayControls)
	{
		int w, h;
		w = MIN (iWidth, iHeight);
		h = w;
		
		cairo_dock_load_image_buffer (&myData.minimizeButton,
			myConfig.cMinimizeImage,
			w, h, CAIRO_DOCK_ANIMATED_IMAGE);
		cairo_dock_image_buffer_set_timelength (&myData.minimizeButton, 3.);  // 4s

		cairo_dock_load_image_buffer (&myData.maximizeButton,
			myConfig.cMaximizeImage,
			w, h, CAIRO_DOCK_ANIMATED_IMAGE);
		cairo_dock_image_buffer_set_timelength (&myData.maximizeButton, 3.);

		cairo_dock_load_image_buffer (&myData.restoreButton,
			myConfig.cRestoreImage,
			w, h, CAIRO_DOCK_ANIMATED_IMAGE);
		cairo_dock_image_buffer_set_timelength (&myData.restoreButton, 3.);

		cairo_dock_load_image_buffer (&myData.closeButton,
			myConfig.cCloseImage,
			w, h, CAIRO_DOCK_ANIMATED_IMAGE);
		cairo_dock_image_buffer_set_timelength (&myData.closeButton, 3.);
	}
}


void cd_app_menu_default_image (void)
{
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	g_return_if_fail (iHeight != 0);
	
	int w, h;
	if (myConfig.bDisplayControls)
	{
		w = MIN (iWidth, iHeight);
		h = w;
	}
	else
	{
		w = iWidth;
		h = iHeight;
	}
	cairo_dock_load_image_buffer (&myData.defaultIcon,
		MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
		w, h, 0);
}


void cd_app_menu_redraw_icon (void)
{
	// load the buttons and the default icon
	if (myData.pCurrentWindow == NULL && myData.defaultIcon.iWidth == 0)
	{
		cd_app_menu_default_image ();
	}
	
	if (myData.minimizeButton.iWidth == 0)
	{
		cd_app_menu_load_button_images ();
	}
	
	// set and launch a transition
	CD_APPLET_SET_TRANSITION_ON_MY_ICON (cd_app_menu_render_step_cairo,
		cd_app_menu_render_step_opengl,
		g_bUseOpenGL,  // bFastPace : vite si opengl, lent si cairo.
		myConfig.iTransitionDuration,
		TRUE);  // bRemoveWhenFinished
}


void cd_app_menu_redraw_buttons (void)
{
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();
		cd_app_menu_render_step_opengl (myIcon, myApplet);	
		CD_APPLET_FINISH_DRAWING_MY_ICON;
	}
	else
	{
		cd_app_menu_render_step_cairo (myIcon, myApplet);
	}
}


void cd_app_menu_resize (void)
{
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	if (myContainer->bIsHorizontal)
		cairo_dock_resize_applet (myApplet, MAX (iWidth, myData.iNbButtons*iHeight), iHeight);
	else
		cairo_dock_resize_applet (myApplet, iWidth, MAX (myData.iNbButtons*iWidth, iHeight));
}


CDButtonEnum cd_app_menu_find_button (GldiModuleInstance *myApplet)
{
	int iNumButton = -1;
	int iMouseX, iMouseY;
	if (myDesklet)  /// TODO: handle the opengl picking...
	{
		iMouseX = myDesklet->iMouseX2d;
		iMouseY = myDesklet->iMouseY2d;
	}
	else
	{
		iMouseX = myContainer->iMouseX - myIcon->fDrawX;
		iMouseY = myContainer->iMouseY - myIcon->fDrawY;
	}
	
	int w, h;
	if (myContainer->bIsHorizontal)
	{
		w = myIcon->fWidth * myIcon->fScale;
		h = myIcon->fHeight * myIcon->fScale;
	}
	else
	{
		h = myIcon->fWidth * myIcon->fScale;
		w = myIcon->fHeight * myIcon->fScale;
		int tmp = iMouseX;
		iMouseX = iMouseY;
		iMouseY = tmp;
	}
	g_return_val_if_fail (w + h != 0, iNumButton); // it can crash with Arithmetic exception if we switch from the dock to a desklet
	if (w >= h)  // horizontal alignment
	{
		iNumButton = iMouseX / (w/myData.iNbButtons);
	}
	else  // vertical alignment
	{
		iNumButton = iMouseY / (h/myData.iNbButtons);
	}
	
	if (!myConfig.bDisplayControls)
		iNumButton++;

	if (myData.bReversedButtonsOrder) // 1->1 ; 2->4 ; 3->2 ; 4->3
	{	// Menu (1) - Close (4) - Min (2) - Max (3) instead of
		// Menu (1) - Min (2) - Max (3) - Close (4)
		if (iNumButton == CD_BUTTON_MINIMIZE) // 2 is now 4
			iNumButton = CD_BUTTON_CLOSE;
		else if (iNumButton != CD_BUTTON_MENU)
			iNumButton--; // 3 is now 2 and 4 is now 3
	}
	
	return iNumButton;
}
