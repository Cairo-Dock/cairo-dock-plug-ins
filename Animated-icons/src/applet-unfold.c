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
	cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pDock), &w, &h);
	double f = 1. - pIcon->pSubDock->fFoldingFactor;
	double fMaxScale = cairo_dock_get_max_scale (CAIRO_CONTAINER (pDock));
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
	/**cairo_set_source_surface (pCairoContext,
		g_pBoxBelowBuffer.pSurface,
		0.,
		0.);
	cairo_paint (pCairoContext);*/
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
	cairo_scale(pCairoContext,
		.8,
		.8);
	int i;
	double dx, dy;
	Icon *icon;
	GList *ic;
	for (ic = pIcon->pSubDock->icons, i = 0; ic != NULL && i < 3; ic = ic->next, i++)
	{
		icon = ic->data;
		if (CAIRO_DOCK_IS_SEPARATOR (icon))
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
		cairo_set_source_surface (pCairoContext,
			icon->pIconBuffer,
			dx,
			dy);
		cairo_paint_with_alpha (pCairoContext, 1. - f);
	}
	cairo_restore (pCairoContext);
	
	//\______________ On dessine la boite devant.
	cairo_save (pCairoContext);
	cairo_scale(pCairoContext,
		(double) w / g_pBoxAboveBuffer.iWidth,
		(double) h / g_pBoxAboveBuffer.iHeight);
	/**cairo_set_source_surface (pCairoContext,
		g_pBoxAboveBuffer.pSurface,
		0.,
		0.);
	cairo_paint (pCairoContext);*/
	cairo_dock_draw_surface (pCairoContext,
		g_pBoxAboveBuffer.pSurface,
		g_pBoxAboveBuffer.iWidth, g_pBoxAboveBuffer.iHeight,
		pDock->container.bDirectionUp,
		pDock->container.bIsHorizontal,
		1.);
	cairo_restore (pCairoContext);
	
	//\_____________________ On dessine son reflet.
	if (pDock->container.bUseReflect && pIcon->pReflectionBuffer != NULL)  // on dessine les reflets.
	{
		cairo_save (pCairoContext);
		double fRatio = pDock->container.fRatio;
		if (pDock->container.bIsHorizontal)
		{
			if (myIconsParam.bConstantSeparatorSize && CAIRO_DOCK_IS_SEPARATOR (pIcon))
				cairo_translate (pCairoContext, 0, (pDock->container.bDirectionUp ? pIcon->fDeltaYReflection + pIcon->fHeight : -pIcon->fDeltaYReflection - myIconsParam.fReflectSize * fRatio));
			else
				cairo_translate (pCairoContext, 0, (pDock->container.bDirectionUp ? pIcon->fDeltaYReflection + pIcon->fHeight * pIcon->fScale : -pIcon->fDeltaYReflection - myIconsParam.fReflectSize * pIcon->fScale * fRatio));
		}
		else
		{
			if (myIconsParam.bConstantSeparatorSize && CAIRO_DOCK_IS_SEPARATOR (pIcon))
				cairo_translate (pCairoContext, (pDock->container.bDirectionUp ? pIcon->fDeltaYReflection + pIcon->fHeight : -pIcon->fDeltaYReflection - myIconsParam.fReflectSize * fRatio), 0);
			else
				cairo_translate (pCairoContext, (pDock->container.bDirectionUp ? pIcon->fDeltaYReflection + pIcon->fHeight * pIcon->fScale : -pIcon->fDeltaYReflection - myIconsParam.fReflectSize * pIcon->fScale * fRatio), 0);
		}
		cairo_dock_set_icon_scale_on_context (pCairoContext, pIcon, pDock->container.bIsHorizontal, fRatio, pDock->container.bDirectionUp);
		
		cairo_set_source_surface (pCairoContext, pIcon->pReflectionBuffer, 0.0, 0.0);
		
		if (myBackendsParam.bDynamicReflection && pIcon->fScale > 1)  // on applique la surface avec un degrade en transparence, ou avec une transparence simple.
		{
			cairo_pattern_t *pGradationPattern;
			if (pDock->container.bIsHorizontal)
			{
				pGradationPattern = cairo_pattern_create_linear (0.,
					(pDock->container.bDirectionUp ? 0. : myIconsParam.fReflectSize / fRatio * (1 + myIconsParam.fAmplitude)),
					0.,
					(pDock->container.bDirectionUp ? myIconsParam.fReflectSize / fRatio * (1 + myIconsParam.fAmplitude) / pIcon->fScale : myIconsParam.fReflectSize / fRatio * (1 + myIconsParam.fAmplitude) * (1. - 1./ pIcon->fScale)));  // de haut en bas.
				g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
				
				cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					0.,
					0.,
					0.,
					0.,
					1.);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					1.,
					0.,
					0.,
					0.,
					1 - (pIcon->fScale - 1) / myIconsParam.fAmplitude);  // astuce pour ne pas avoir a re-creer la surface de la reflection.
			}
			else
			{
				pGradationPattern = cairo_pattern_create_linear ((pDock->container.bDirectionUp ? 0. : myIconsParam.fReflectSize / fRatio * (1 + myIconsParam.fAmplitude)),
					0.,
					(pDock->container.bDirectionUp ? myIconsParam.fReflectSize / fRatio * (1 + myIconsParam.fAmplitude) / pIcon->fScale : myIconsParam.fReflectSize / fRatio * (1 + myIconsParam.fAmplitude) * (1. - 1./ pIcon->fScale)),
					0.);
				g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
				
				cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					0.,
					0.,
					0.,
					0.,
					1.);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					1.,
					0.,
					0.,
					0.,
					1. - (pIcon->fScale - 1) / myIconsParam.fAmplitude);  // astuce pour ne pas avoir a re-creer la surface de la reflection.
			}
			cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
			cairo_translate (pCairoContext, 0, 0);
			cairo_mask (pCairoContext, pGradationPattern);

			cairo_pattern_destroy (pGradationPattern);
		}
		else
		{
			if (pIcon->fAlpha == 1)
				cairo_paint (pCairoContext);
			else
				cairo_paint_with_alpha (pCairoContext, pIcon->fAlpha);
		}
		cairo_restore (pCairoContext);
	}
	cairo_restore (pCairoContext);
}


void cd_animations_draw_unfolding_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	g_return_if_fail (pIcon->pSubDock != NULL);
	int w, h;
	cairo_dock_get_icon_extent (pIcon, CAIRO_CONTAINER (pDock), &w, &h);
	double f = 1. - pIcon->pSubDock->fFoldingFactor;
	double fMaxScale = cairo_dock_get_max_scale (CAIRO_CONTAINER (pDock));
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
	double dx = 0., dy = 0.;
	Icon *icon;
	GList *ic;
	for (ic = pIcon->pSubDock->icons, i = 0; ic != NULL && i < 3; ic = ic->next, i++)
	{
		icon = ic->data;
		if (CAIRO_DOCK_IS_SEPARATOR (icon))
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
}
