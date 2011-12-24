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

#include "applet-struct.h"
#include "applet-draw.h"


static gboolean cd_app_menu_render_step_opengl (Icon *pIcon, CairoDockModuleInstance *myApplet)
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
	Icon *pAppli = cairo_dock_get_icon_with_Xid (myData.iCurrentWindow);
	GLuint iTexture = (pAppli ? pAppli->iIconTexture : myData.defaultIcon.iTexture);
	Icon *pPrevIcon = cairo_dock_get_icon_with_Xid (myData.iPreviousWindow);
	GLuint iPrevTexture = (pPrevIcon ? pPrevIcon->iIconTexture : myData.defaultIcon.iTexture);
	
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
	
	// draw window controls
	if (myConfig.bDisplayControls)
	{
		// minimize button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y -= w;
		if (myData.bCanMinimize)
			_cairo_dock_set_alpha (1);
		else
			_cairo_dock_set_alpha (.6);
		
		glBindTexture (GL_TEXTURE_2D, myData.minimizeButton.iTexture);
		_cairo_dock_apply_current_texture_at_size_with_offset (w, h, x, y);
		
		// restore/maximize button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y -= w;
		if (myData.bCanMaximize)
			_cairo_dock_set_alpha (1);
		else
			_cairo_dock_set_alpha (.6);
		
		glBindTexture (GL_TEXTURE_2D, pAppli && pAppli->bIsMaximized ? myData.restoreButton.iTexture : myData.maximizeButton.iTexture);
		_cairo_dock_apply_current_texture_at_size_with_offset (w, h, x, y);
		
		// close button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y -= w;
		if (myData.bCanClose)
			_cairo_dock_set_alpha (1);
		else
			_cairo_dock_set_alpha (.6);
		
		glBindTexture (GL_TEXTURE_2D, myData.closeButton.iTexture);
		_cairo_dock_apply_current_texture_at_size_with_offset (w, h, x, y);
	}
	_cairo_dock_disable_texture ();
	
	CD_APPLET_LEAVE (TRUE);
}

static gboolean cd_app_menu_render_step_cairo (Icon *pIcon, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	double f = CD_APPLET_GET_TRANSITION_FRACTION ();
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	CD_APPLET_LEAVE_IF_FAIL (iHeight != 0, TRUE);
	
	cairo_dock_erase_cairo_context (myDrawContext);
	
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
	
	Icon *pAppli = cairo_dock_get_icon_with_Xid (myData.iCurrentWindow);
	cairo_surface_t *pSurface = (pAppli ? pAppli->pIconBuffer : myData.defaultIcon.pSurface);
	Icon *pPrevIcon = cairo_dock_get_icon_with_Xid (myData.iPreviousWindow);
	cairo_surface_t *pPrevSurface = (pPrevIcon ? pPrevIcon->pIconBuffer : myData.defaultIcon.pSurface);
	
	if (pSurface != NULL)
	{
		cairo_set_source_surface (myDrawContext, pSurface, x, y);
		cairo_paint_with_alpha (myDrawContext, 1-f);
	}
	if (pPrevSurface != NULL)
	{
		cairo_set_source_surface (myDrawContext, pPrevSurface, x, y);
		cairo_paint_with_alpha (myDrawContext, f);
	}
	
	// draw window controls
	if (myConfig.bDisplayControls)
	{
		// minimize button
		if (iWidth > iHeight)  // horizontal alignment
			x += w;
		else
			y += w;
		
		
		
	}
	
	CD_APPLET_LEAVE (TRUE);
}


void cd_app_menu_redraw_icon (void)
{
	// load the buttons and the default icon
	if (myData.iCurrentWindow == 0 && myData.defaultIcon.iWidth == 0)
	{
		
	}
	
	if (myData.minimizeButton.iWidth == 0)
	{
		
	}
	
	// set and launch a transition
	CD_APPLET_SET_TRANSITION_ON_MY_ICON (cd_app_menu_render_step_cairo,
		cd_app_menu_render_step_opengl,
		g_bUseOpenGL,  // bFastPace : vite si opengl, lent si cairo.
		myConfig.iTransitionDuration,
		TRUE);  // bRemoveWhenFinished
	/*if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();
		cd_app_menu_render_step_opengl (myIcon, myApplet);	
		CD_APPLET_FINISH_DRAWING_MY_ICON;
	}
	else
	{
		cd_app_menu_render_step_cairo (myIcon, myApplet);
		CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
	}
	CD_APPLET_REDRAW_MY_ICON;*/
}
