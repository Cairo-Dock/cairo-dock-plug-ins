/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

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
	
	if (myData.bWink && myData.iEyelidTexture)
	{
		_cairo_dock_apply_texture_at_size (myData.iToonTexture,
			myData.iToonWidth,
			myData.iToonHeight);
		
		_cairo_dock_set_blend_pbuffer ();
		
		glPushMatrix ();
		glTranslatef (-.5*iWidth + myData.iXeyelid + .5*myData.iEyelidWidth,
			.5*iHeight - myData.iYeyelid - .5*myData.iEyelidHeight,
			0.);
		_cairo_dock_apply_texture_at_size (myData.iEyelidTexture,
			myData.iEyelidWidth,
			myData.iEyelidHeight);
		glPopMatrix ();
	}
	else
	{
		if (myData.iBgTexture)
		{
			_cairo_dock_set_blend_source ();
			glPushMatrix ();
			glTranslatef (-.5*iWidth + myData.iXbg + .5*myData.iBgWidth,
				.5*iHeight - myData.iYbg - .5*myData.iBgHeight,
				0.);
			_cairo_dock_apply_texture_at_size (myData.iBgTexture,
				myData.iBgWidth,
				myData.iBgHeight);
			glPopMatrix ();
		}
		_cairo_dock_set_blend_alpha ();
		int i;
		for (i = 0; i < 2; i ++)
		{
			if (myData.iPupilTexture[i] != 0)
			{
				glPushMatrix ();
				glTranslatef (-.5*iWidth + myData.fXpupil[i],
					.5*iHeight - myData.fYpupil[i],
					0.);
				_cairo_dock_apply_texture_at_size (myData.iPupilTexture[i],
					myData.iPupilWidth[i],
					myData.iPupilHeight[i]);
				glPopMatrix ();
			}
		}
		_cairo_dock_set_blend_pbuffer ();
		_cairo_dock_apply_texture_at_size (myData.iToonTexture,
			myData.iToonWidth,
			myData.iToonHeight);
	}
	
	_cairo_dock_disable_texture ();
	
	CD_APPLET_FINISH_DRAWING_MY_ICON;
}


void cd_xeyes_render_to_surface (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	cairo_dock_erase_cairo_context (myDrawContext);
	
	
	if (myData.bWink && myData.pEyelidSurface)
	{
		cairo_set_source_surface (myDrawContext,
			myData.pToonSurface,
			.5*(iWidth - myData.iToonWidth),
			.5*(iHeight - myData.iToonHeight));
		cairo_paint (myDrawContext);
		
		cairo_set_source_surface (myDrawContext,
			myData.pEyelidSurface,
			myData.iXeyelid,
			myData.iYeyelid);
		cairo_paint (myDrawContext);
	}
	else
	{
		if (myData.pBgSurface)
		{
			cairo_set_source_surface (myDrawContext,
				myData.pBgSurface,
				myData.iXbg,
				myData.iYbg);
			cairo_paint (myDrawContext);
		}
		
		int i;
		for (i = 0; i < 2; i ++)
		{
			if (myData.pPupilSurface[i])
			{
				cairo_set_source_surface (myDrawContext,
					myData.pPupilSurface[i],
					myData.fXpupil[i] - .5*myData.iPupilWidth[i],
					myData.fYpupil[i] - .5*myData.iPupilHeight[i]);
				cairo_paint (myDrawContext);
			}
		}
		cairo_set_source_surface (myDrawContext,
			myData.pToonSurface,
			.5*(iWidth - myData.iToonWidth),
			.5*(iHeight - myData.iToonHeight));
	}
	
	cairo_paint (myDrawContext);
	
	CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
}
