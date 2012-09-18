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
#include <math.h>

#include "applet-struct.h"
#include "applet-bounce.h"


void cd_animations_draw_unfolding_icon_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	g_return_if_fail (pIcon->pSubDock != NULL && pIcon->pIconBuffer != NULL);
	int w, h;
	cairo_dock_get_icon_extent (pIcon, &w, &h);
	double f = 1. - pIcon->pSubDock->fFoldingFactor;
	double fMaxScale = (pIcon->fHeight != 0 ? (pDock->container.bIsHorizontal ? pIcon->iImageHeight : pIcon->iImageWidth) / pIcon->fHeight : 1.);
	double z = pIcon->fScale / fMaxScale * pDock->container.fRatio;
	
	//\______________ On dessine la boite derriere.
	cairo_save (pCairoContext);
	cairo_scale (pCairoContext, z, z);
	if (g_pIconBackgroundBuffer.pSurface != NULL)  // on ecrase le dessin existant avec l'image de fond des icones.
	{
		cairo_save (pCairoContext);
		cairo_scale(pCairoContext,
			(double) w / g_pIconBackgroundBuffer.iWidth,
			(double) h / g_pIconBackgroundBuffer.iHeight);
		cairo_set_source_surface (pCairoContext,
			g_pIconBackgroundBuffer.pSurface,
			0.,
			0.);
		cairo_paint (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	cairo_save (pCairoContext);
	cairo_scale(pCairoContext,
		(double) w / g_pBoxBelowBuffer.iWidth,
		(double) h / g_pBoxBelowBuffer.iHeight);
	cairo_dock_draw_surface (pCairoContext,
		g_pBoxBelowBuffer.pSurface,
		g_pBoxBelowBuffer.iWidth, g_pBoxBelowBuffer.iHeight,
		pDock->container.bDirectionUp,
		pDock->container.bIsHorizontal,
		1.);
	cairo_restore (pCairoContext);
	
	//\______________ On dessine les 3 premieres icones du sous-dock.
	cairo_save (pCairoContext);
	if (pDock->container.bIsHorizontal)
	{
		if (!pDock->container.bDirectionUp)
			cairo_translate (pCairoContext, 0., .2*h);
	}
	else
	{
		if (! pDock->container.bDirectionUp)
			cairo_translate (pCairoContext, .2*h, 0.);
	}
	int i;
	double dx, dy;
	int wi, hi;
	Icon *icon;
	GList *ic;
	for (ic = pIcon->pSubDock->icons, i = 0; ic != NULL && i < 3; ic = ic->next, i++)
	{
		icon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
		{
			i --;
			continue;
		}
		
		if (pDock->container.bIsHorizontal)
		{
			dx = .1*w;
			if (pDock->container.bDirectionUp)
				dy = (.1*i - f*1.5) * h/z;
			else
				dy = - (.1*i - f*1.5) * h/z;
		}
		else
		{
			dy = .1*w;
			if (pDock->container.bDirectionUp)
				dx = (.1*i - f*1.5) * h/z;
			else
				dx = - (.1*i - f*1.5) * h/z;
		}
		
		cairo_dock_get_icon_extent (icon, &wi, &hi);
		
		cairo_save (pCairoContext);
		cairo_translate (pCairoContext, dx, dy);
		cairo_scale (pCairoContext, .8 * w / wi, .8 * h / hi);
		cairo_set_source_surface (pCairoContext,
			icon->pIconBuffer,
			0,
			0);
		cairo_paint_with_alpha (pCairoContext, 1. - f);
		cairo_restore (pCairoContext);
	}
	cairo_restore (pCairoContext);
	
	//\______________ On dessine la boite devant.
	cairo_save (pCairoContext);
	cairo_scale(pCairoContext,
		(double) w / g_pBoxAboveBuffer.iWidth,
		(double) h / g_pBoxAboveBuffer.iHeight);
	cairo_dock_draw_surface (pCairoContext,
		g_pBoxAboveBuffer.pSurface,
		g_pBoxAboveBuffer.iWidth, g_pBoxAboveBuffer.iHeight,
		pDock->container.bDirectionUp,
		pDock->container.bIsHorizontal,
		1.);
	cairo_restore (pCairoContext);
	
	//\_____________________ On dessine son reflet.
	cairo_restore (pCairoContext);  // come back to the original context
	
	cairo_dock_draw_icon_reflect_cairo (pIcon, CAIRO_CONTAINER (pDock), pCairoContext);
}


void cd_animations_draw_unfolding_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	g_return_if_fail (pIcon->pSubDock != NULL);
	int w, h;
	cairo_dock_get_icon_extent (pIcon, &w, &h);
	double f = 1. - pIcon->pSubDock->fFoldingFactor;
	double fMaxScale = (pIcon->fHeight != 0 ? (pDock->container.bIsHorizontal ? pIcon->iImageHeight : pIcon->iImageWidth) / pIcon->fHeight : 1.);
	double z = pIcon->fScale / fMaxScale * pDock->container.fRatio;
	
	//\______________ On dessine la boite derriere.
	glPushMatrix ();
	if (pDock->container.bIsHorizontal)
	{
		if (! pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	else
	{
		glRotatef (90., 0., 0., 1.);
		if (!pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	glScalef (z, z, 1.);
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	if (g_pIconBackgroundBuffer.iTexture != 0)  // on ecrase le dessin existant avec l'image de fond des icones.
	{
		_cairo_dock_apply_texture_at_size (g_pIconBackgroundBuffer.iTexture, w, h);
	}
	
	_cairo_dock_apply_texture_at_size (g_pBoxBelowBuffer.iTexture, w, h);
	
	//\______________ On dessine les 3 premieres icones du sous-dock.
	glMatrixMode(GL_TEXTURE);
	glPushMatrix ();
	if (pDock->container.bIsHorizontal)
	{
		if (! pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	else
	{
		glRotatef (-90., 0., 0., 1.);
		if (!pDock->container.bDirectionUp)
			glScalef (1., -1., 1.);
	}
	glMatrixMode (GL_MODELVIEW);
	_cairo_dock_set_alpha (sqrt (MAX (0., 1. - f)));  // on reduit un peu la transparence en opengl.
	int i;
	Icon *icon;
	GList *ic;
	for (ic = pIcon->pSubDock->icons, i = 0; ic != NULL && i < 3; ic = ic->next, i++)
	{
		icon = ic->data;
		if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
		{
			i --;
			continue;
		}
		glBindTexture (GL_TEXTURE_2D, icon->iIconTexture);
		_cairo_dock_apply_current_texture_at_size_with_offset (.8*w,
			.8*h,
			0.,
			(.1*(1-i) + f) * h/z);
	}
	glMatrixMode(GL_TEXTURE);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	
	//\______________ On dessine la boite devant.
	_cairo_dock_set_alpha (1.);
	_cairo_dock_apply_texture_at_size (g_pBoxAboveBuffer.iTexture, w, h);
	glPopMatrix ();
	
	//\_____________________ On dessine son reflet.
	cairo_dock_draw_icon_reflect_opengl (pIcon, pDock);
	
	_cairo_dock_disable_texture ();
}
