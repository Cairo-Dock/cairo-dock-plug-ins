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
#include "applet-draw.h"


void cd_xeyes_render_to_texture (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	if (myData.bWink && myData.pEyelidImage)
	{
		cairo_dock_apply_image_buffer_texture (myData.pToonImage);
		
		_cairo_dock_set_blend_pbuffer ();
		
		cairo_dock_apply_image_buffer_texture_with_offset (myData.pEyelidImage,
			-.5*iWidth + myData.iXeyelid + .5*myData.pEyelidImage->iWidth,
			.5*iHeight - myData.iYeyelid - .5*myData.pEyelidImage->iHeight);
	}
	else
	{
		if (myData.pBgImage)
		{
			_cairo_dock_set_blend_source ();
			cairo_dock_apply_image_buffer_texture_with_offset (myData.pBgImage,
				-.5*iWidth + myData.iXbg + .5*myData.pBgImage->iWidth,
				.5*iHeight - myData.iYbg - .5*myData.pBgImage->iHeight);
		}
		_cairo_dock_set_blend_alpha ();
		int i;
		for (i = 0; i < 2; i ++)
		{
			if (myData.pPupilImage[i] != 0)
			{
				cairo_dock_apply_image_buffer_texture_with_offset (myData.pPupilImage[i],
					-.5*iWidth + myData.fXpupil[i],
					.5*iHeight - myData.fYpupil[i]);
			}
		}
		_cairo_dock_set_blend_pbuffer ();
		cairo_dock_apply_image_buffer_texture (myData.pToonImage);
	}
	
	_cairo_dock_disable_texture ();
	
	CD_APPLET_FINISH_DRAWING_MY_ICON;
}


void cd_xeyes_render_to_surface (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO ();
	///cairo_dock_erase_cairo_context (myDrawContext);
	if (myData.bWink && myData.pEyelidImage)
	{
		cairo_dock_apply_image_buffer_surface_with_offset (myData.pToonImage, myDrawContext,
			.5*(iWidth - myData.pToonImage->iWidth),
			.5*(iHeight - myData.pToonImage->iHeight),
			1.);
		
		cairo_dock_apply_image_buffer_surface_with_offset (myData.pEyelidImage, myDrawContext,
			myData.iXeyelid,
			myData.iYeyelid,
			1.);	
	}
	else
	{
		if (myData.pBgImage)
		{
			cairo_dock_apply_image_buffer_surface_with_offset (myData.pBgImage, myDrawContext,
				myData.iXbg,
				myData.iYbg,
				1.);
		}
		
		int i;
		for (i = 0; i < 2; i ++)
		{
			if (myData.pPupilImage[i])
			{
				cairo_dock_apply_image_buffer_surface_with_offset (myData.pPupilImage[i], myDrawContext,
					myData.fXpupil[i] - .5*myData.pPupilImage[i]->iWidth,
					myData.fYpupil[i] - .5*myData.pPupilImage[i]->iHeight,
					1.);
			}
		}
		cairo_dock_apply_image_buffer_surface_with_offset (myData.pToonImage, myDrawContext,
			.5*(iWidth - myData.pToonImage->iWidth),
			.5*(iHeight - myData.pToonImage->iHeight),
			1.);
	}
	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
}
