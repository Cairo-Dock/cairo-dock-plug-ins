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
#include "applet-session.h"
#include "applet-draw.h"

#define STATIC_ANGLE 20.

#define _alpha_prompt(k,n) cos (G_PI/2*fabs ((double) ((k % (2*n)) - n) / n));

const int s_iNbPromptAnimationSteps = 40;

static inline int _cd_do_get_matching_icons_width (int *iNbIcons)
{
	int i = 0, iIconsWidth = 0;
	Icon *pIcon;
	int iWidth, iHeight;
	double fZoom;
	GList *ic;
	for (ic = myData.pMatchingIcons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->image.pSurface == NULL && pIcon->image.iTexture == 0)  // icone pas encore chargee.
			continue;
		cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
		if (iHeight != 0)
		{
			fZoom = (double) g_pMainDock->container.iHeight/2 / iHeight;
			iIconsWidth += iWidth * fZoom;
		}
		i ++;
	}
	*iNbIcons = i;
	return iIconsWidth;
}

static inline int _cd_do_get_icon_x (GList *pElement)
{
	int iOffset = 0;
	Icon *pIcon;
	int iWidth, iHeight;
	double fZoom;
	GList *ic;
	for (ic = myData.pMatchingIcons; ic != NULL && ic != pElement; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->image.pSurface == NULL && pIcon->image.iTexture == 0)  // icone pas encore chargee.
			continue;
		cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
		if (iHeight != 0)
		{
			fZoom = (double) g_pMainDock->container.iHeight/2 / iHeight;
			iOffset += iWidth * fZoom;
		}
	}
	return iOffset;
}

void cd_do_render_cairo (CairoDock *pMainDock, cairo_t *pCairoContext)
{
	double fAlpha;
	if (myData.iCloseTime != 0) // animation de fin
		fAlpha = (double) myData.iCloseTime / myConfig.iCloseDuration;
	else
		fAlpha = 1.;
	
	if (myData.pCharList == NULL && myData.pListingHistory == NULL)  // aucune lettre de tapee => on montre le prompt.
	{
		if (myData.pPromptSurface != NULL)
		{
			double fFrameWidth = myData.iPromptWidth;
			double fFrameHeight = myData.iPromptHeight;
			
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
			fDockOffsetX = (pMainDock->container.iWidth - fFrameWidth) / 2;
			fDockOffsetY = (pMainDock->container.iHeight - fFrameHeight) / 2;  // centre verticalement.
			
			fAlpha *= _alpha_prompt (myData.iPromptAnimationCount, s_iNbPromptAnimationSteps);
			
			if (fAlpha != 0)
			{
				cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);
				cairo_dock_draw_surface (pCairoContext, myData.pPromptSurface, fFrameWidth, fFrameHeight, pMainDock->container.bDirectionUp, pMainDock->container.bIsHorizontal, fAlpha);
				//cairo_set_source_surface (pCairoContext, myData.pPromptSurface, 0., 0.);
				//cairo_paint_with_alpha (pCairoContext, fAlpha);
			}
		}
	}
	else  // si du texte a ete entre, on le dessine, ainsi que eventuellement la liste des icones correspondantes.
	{
		// dessin des icones correspondantes.
		int iIconsWidth = 0, iNbIcons = 0;
		if (myData.pMatchingIcons != NULL)
		{
			// on determine au prealable la largeur des icones pour pouvoir les centrer.
			iIconsWidth = _cd_do_get_matching_icons_width (&iNbIcons);
			
			// dessin du fond des icones.
			double fFrameWidth = iIconsWidth;
			double fFrameHeight = pMainDock->container.iHeight/2;
			double fRadius = fFrameHeight / 10;
			double fLineWidth = 0.;
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
			fDockOffsetX = (pMainDock->container.iWidth - fFrameWidth) / 2;
			fDockOffsetY = (!myConfig.bTextOnTop ? 0. : pMainDock->container.iHeight - fFrameHeight);
			
			cairo_save (pCairoContext);
			cairo_translate (pCairoContext, fDockOffsetX -fRadius, fDockOffsetY);
			cairo_dock_draw_rounded_rectangle (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight);
			cairo_set_line_width (pCairoContext, fLineWidth);
			cairo_set_source_rgba (pCairoContext, myConfig.pFrameColor[0], myConfig.pFrameColor[1], myConfig.pFrameColor[2], myConfig.pFrameColor[3] * fAlpha);
			cairo_fill (pCairoContext);
			cairo_restore (pCairoContext);
			
			// on les dessine.
			//int iOffsetX = _cd_do_get_icon_x (myData.pCurrentMatchingElement) + myData.iCurrentMatchingOffset * myData.iCurrentMatchingDirection - iIconsWidth/2;  // ecart au centre du debut de l'icone courante.
			//iOffsetX = - iOffsetX;
			int iOffsetX = myData.iCurrentMatchingOffset + iIconsWidth/2;
			while (iOffsetX > iIconsWidth)
				iOffsetX -= iIconsWidth;
			while (iOffsetX < 0)
				iOffsetX += iIconsWidth;
			
			double fIconScale = (iIconsWidth > pMainDock->container.iWidth ? (double) pMainDock->container.iWidth / iIconsWidth : 1.);
			double x0 = (pMainDock->container.iWidth - iIconsWidth * fIconScale) / 2;
			double x = x0 + iOffsetX * fIconScale;  // abscisse de l'icone courante.
			Icon *pIcon;
			int iWidth, iHeight;
			double fZoom;
			GList *ic;
			for (ic = myData.pMatchingIcons; ic != NULL; ic = ic->next)
			{
				pIcon = ic->data;
				if (pIcon->image.pSurface == NULL)  // icone pas encore chargee.
					continue;
				cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
				fZoom = fIconScale * pMainDock->container.iHeight/2 / iHeight * (myData.pCurrentMatchingElement == ic ? 1. : 1.);
				cairo_save (pCairoContext);
				
				//if (!bRound)
				{
					if (x < x0)
						x += iIconsWidth * fIconScale;
					else if (x > x0 + iIconsWidth * fIconScale)
						x -= iIconsWidth * fIconScale;
				}
				if (pMainDock->container.bIsHorizontal)
					cairo_translate (pCairoContext,
						x - (iNbIcons & 1 ? iWidth * fZoom * fIconScale / 2 : 0.),
						(myConfig.bTextOnTop ?
							pMainDock->container.iHeight/2 :
							0.));
				else
					cairo_translate (pCairoContext,
						(myConfig.bTextOnTop ?
							pMainDock->container.iHeight/2 :
							0.),
						x - (iNbIcons & 1 ? iWidth * fZoom * fIconScale / 2 : 0.));
				cairo_scale (pCairoContext,
					fZoom,
					fZoom);
				cairo_set_source_surface (pCairoContext, pIcon->image.pSurface, 0., 0.);
				cairo_paint_with_alpha (pCairoContext, (myData.pCurrentMatchingElement == ic ? 1. : .5));
				
				if (myData.pCurrentMatchingElement == ic)
				{
					fLineWidth = 4.;
					cairo_dock_draw_rounded_rectangle (pCairoContext, fRadius, fLineWidth, iWidth-2*fRadius, iHeight-fLineWidth);
					cairo_set_line_width (pCairoContext, fLineWidth);
					cairo_set_source_rgba (pCairoContext, myConfig.pFrameColor[0]+.1, myConfig.pFrameColor[1]+.1, myConfig.pFrameColor[2]+.1, 1.);
					cairo_stroke (pCairoContext);
				}
				
				cairo_restore (pCairoContext);
				x += iWidth * fZoom;
			}
		}
		
		// dessin du fond du texte.
		double fFrameWidth = myData.iTextWidth;
		double fTextScale = (fFrameWidth > pMainDock->container.iWidth ? (double) pMainDock->container.iWidth / fFrameWidth : 1.);
		double fFrameHeight = myData.iTextHeight;
		double fRadius = fFrameHeight / 5 * myConfig.fFontSizeRatio;
		double fLineWidth = 0.;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = (pMainDock->container.iWidth - fFrameWidth * fTextScale) / 2;
		fDockOffsetY = (myConfig.bTextOnTop ? 0. : pMainDock->container.iHeight - fFrameHeight);
		
		cairo_save (pCairoContext);
		cairo_translate (pCairoContext, fDockOffsetX -fRadius, fDockOffsetY);
		cairo_dock_draw_rounded_rectangle (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight);
		cairo_set_line_width (pCairoContext, fLineWidth);
		cairo_set_source_rgba (pCairoContext, myConfig.pFrameColor[0], myConfig.pFrameColor[1], myConfig.pFrameColor[2], myConfig.pFrameColor[3] * fAlpha);
		cairo_fill (pCairoContext);
		cairo_restore (pCairoContext);
		
		// dessin des lettres.
		CDChar *pChar;
		GList *c;
		for (c = myData.pCharList; c != NULL; c = c->next)
		{
			pChar = c->data;
			cairo_save (pCairoContext);
			
			cairo_translate (pCairoContext,
				(pChar->iCurrentX * fTextScale + pMainDock->container.iWidth/2),
				pChar->iCurrentY + (myConfig.bTextOnTop ?
					(myData.iTextHeight - pChar->iHeight) :
					pMainDock->container.iHeight - pChar->iHeight));  // aligne en bas.
			
			if (fTextScale != 1)
				cairo_scale (pCairoContext, fTextScale, fTextScale);
			cairo_set_source_surface (pCairoContext, pChar->pSurface, 0., 0.);
			cairo_paint_with_alpha (pCairoContext, fAlpha);
			
			cairo_restore (pCairoContext);
		}
	}
}


void cd_do_render_opengl (CairoDock *pMainDock)
{
	double fAlpha;
	if (myData.iCloseTime != 0) // animation de fin
		fAlpha = (double) myData.iCloseTime / myConfig.iCloseDuration;
	else
		fAlpha = 1.;
	
	if (myData.pCharList == NULL && myData.pListingHistory == NULL)  // aucune lettre de tapee => on montre le prompt.
	{
		if (myData.iPromptTexture != 0)
		{
			double fFrameWidth = myData.iPromptWidth;
			double fFrameHeight = myData.iPromptHeight;
			
			/*double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
			fDockOffsetX = (pMainDock->container.iWidth - fFrameWidth) / 2;
			fDockOffsetY = (pMainDock->container.iHeight - fFrameHeight) / 2;*/
			
			fAlpha *= _alpha_prompt (myData.iPromptAnimationCount, s_iNbPromptAnimationSteps);
			
			if (fAlpha != 0)
			{
				glPushMatrix ();
				if (! pMainDock->container.bIsHorizontal)
					glRotatef (pMainDock->container.bDirectionUp ? 90. : -90., 0., 0., 1.);
				glTranslatef (pMainDock->container.iWidth/2, pMainDock->container.iHeight/2, 0.);
				
				_cairo_dock_enable_texture ();
				_cairo_dock_set_blend_alpha ();
				
				_cairo_dock_apply_texture_at_size_with_alpha (myData.iPromptTexture, fFrameWidth, fFrameHeight, fAlpha);
				
				_cairo_dock_disable_texture ();
				
				glPopMatrix();
			}
		}
	}
	else  // si du texte a ete entre, on le dessine, ainsi que eventuellement la liste des icones correspondantes.
	{
		// dessin des icones correspondantes.
		int iIconsWidth = 0, iNbIcons = 0;
		if (myData.pMatchingIcons != NULL)
		{
			// on determine au prealable la largeur des icones pour povouir les centrer.
			iIconsWidth = _cd_do_get_matching_icons_width (&iNbIcons);
			
			// dessin du fond des icones.
			double fFrameWidth = iIconsWidth;
			double fFrameHeight = pMainDock->container.iHeight/2;
			double fRadius = fFrameHeight / 10;
			double fLineWidth = 0.;
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
			fDockOffsetX = pMainDock->container.iWidth/2 - fFrameWidth / 2 - fRadius;
			fDockOffsetY = (!myConfig.bTextOnTop ? pMainDock->container.iHeight : fFrameHeight);
			double fFrameColor[4] = {myConfig.pFrameColor[0], myConfig.pFrameColor[1], myConfig.pFrameColor[2], myConfig.pFrameColor[3] * fAlpha};
			glPushMatrix ();
			if (! pMainDock->container.bIsHorizontal)
				glRotatef (pMainDock->container.bDirectionUp ? 90. : -90., 0., 0., 1.);
			glTranslatef (fDockOffsetX, fDockOffsetY, 0.);
			cairo_dock_draw_rounded_rectangle_opengl (fFrameWidth, fFrameHeight, fRadius, fLineWidth, fFrameColor);
			glPopMatrix ();
			
			// on les dessine.
			//int iOffsetX = _cd_do_get_icon_x (myData.pCurrentMatchingElement) + myData.iCurrentMatchingOffset * myData.iCurrentMatchingDirection - iIconsWidth/2;  // ecart au centre du debut de l'icone courante.
			//iOffsetX = - iOffsetX;
			//g_print ("myData.iCurrentMatchingOffset : %d\n", myData.iCurrentMatchingOffset);
			int iOffsetX = myData.iCurrentMatchingOffset + iIconsWidth/2;
			while (iOffsetX > iIconsWidth)
				iOffsetX -= iIconsWidth;
			while (iOffsetX < 0)
				iOffsetX += iIconsWidth;
			
			double fIconScale = (iIconsWidth > pMainDock->container.iWidth ? (double) pMainDock->container.iWidth / (iIconsWidth) : 1.);
			double x0 = (pMainDock->container.iWidth - iIconsWidth * fIconScale) / 2;
			double x = x0 - iOffsetX * fIconScale;  // abscisse de l'icone courante.
			Icon *pIcon;
			int iWidth, iHeight;
			double fZoom;
			GList *ic;
			
			_cairo_dock_enable_texture ();
			_cairo_dock_set_blend_alpha ();
			_cairo_dock_set_alpha (1.);
			for (ic = myData.pMatchingIcons; ic != NULL; ic = ic->next)
			{
				pIcon = ic->data;
				if (pIcon->image.iTexture == 0)  // icone pas encore chargee.
					continue;
				cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
				fZoom = (double) pMainDock->container.iHeight/2 / iHeight * (myData.pCurrentMatchingElement == ic ? 1. : 1.);
				glPushMatrix ();
				
				//if (!bRound)
				{
					if (x < x0)
						x += iIconsWidth * fIconScale;
					else if (x + (iNbIcons & 1 ? 0. : iWidth * fZoom/2 * fIconScale) > x0 + iIconsWidth * fIconScale)
						x -= iIconsWidth * fIconScale;
				}
				if (pMainDock->container.bIsHorizontal)
					glTranslatef (x + (iNbIcons & 1 ? 0. : iWidth * fZoom/2 * fIconScale),
						(myConfig.bTextOnTop ?
							pMainDock->container.iHeight/4 :
							pMainDock->container.iHeight - iHeight * fZoom/2),
						0.);
				else
					glTranslatef ((myConfig.bTextOnTop ?
							pMainDock->container.iHeight/4 :
							pMainDock->container.iHeight - iHeight * fZoom/2),
						x + (iNbIcons & 1 ? 0. : iWidth * fZoom/2 * fIconScale),
						0.);
				_cairo_dock_apply_texture_at_size_with_alpha (pIcon->image.iTexture,
					iWidth * fZoom * fIconScale,
					pMainDock->container.iHeight/2 * fIconScale,
					(myData.pCurrentMatchingElement == ic ? 1. : .5));
				
				if (myData.pCurrentMatchingElement == ic)
				{
					_cairo_dock_disable_texture ();
					fLineWidth = 4.;
					double fFrameColor[4] = {myConfig.pFrameColor[0]+.1, myConfig.pFrameColor[1]+.1, myConfig.pFrameColor[2]+.1, 1.};
					glTranslatef (iWidth/2 * fZoom * fIconScale + fLineWidth/2, iHeight * fZoom/2 * fIconScale - fLineWidth/2, 0.);
					cairo_dock_draw_rounded_rectangle_opengl (iWidth * fZoom * fIconScale - fLineWidth,
						iHeight * fZoom * fIconScale - fLineWidth,
						fRadius,
						fLineWidth,
						fFrameColor);
					_cairo_dock_enable_texture ();
					_cairo_dock_set_blend_alpha ();
				}
				
				glPopMatrix ();
				x += iWidth * fZoom * fIconScale;
			}
			_cairo_dock_disable_texture ();
		}
		_cairo_dock_set_alpha (1.);
		
		// dessin du fond du texte.
		double fFrameWidth = myData.iTextWidth;
		double fTextScale = (fFrameWidth > pMainDock->container.iWidth ? (double) pMainDock->container.iWidth / fFrameWidth : 1.);
		double fFrameHeight = myData.iTextHeight;
		double fRadius = myDocksParam.iDockRadius * myConfig.fFontSizeRatio;
		
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = pMainDock->container.iWidth/2 - fFrameWidth * fTextScale / 2 - fRadius;
		fDockOffsetY = (myConfig.bTextOnTop ? pMainDock->container.iHeight : fFrameHeight);
		
		glPushMatrix ();
		double fFrameColor[4] = {myConfig.pFrameColor[0], myConfig.pFrameColor[1], myConfig.pFrameColor[2], myConfig.pFrameColor[3] * fAlpha};
		
		if (! pMainDock->container.bIsHorizontal)
			glRotatef (pMainDock->container.bDirectionUp ? 90. : -90., 0., 0., 1.);
		glTranslatef (fDockOffsetX, fDockOffsetY, 0.);
		cairo_dock_draw_rounded_rectangle_opengl (fFrameWidth, fFrameHeight, fRadius, 0, fFrameColor);
		glPopMatrix();
		
		// dessin des lettres.
		_cairo_dock_enable_texture ();
		_cairo_dock_set_blend_alpha ();
		_cairo_dock_set_alpha (fAlpha);
		
		gldi_gl_container_set_perspective_view (CAIRO_CONTAINER (pMainDock));
		if (pMainDock->container.bIsHorizontal)
		{
			glTranslatef (-pMainDock->container.iWidth/2, -pMainDock->container.iHeight/2, 0.);
		}
		else
		{
			glTranslatef (-pMainDock->container.iHeight/2, -pMainDock->container.iWidth/2, 0.);
		}
		glEnable (GL_DEPTH_TEST);
		
		CDChar *pChar;
		GList *c;
		for (c = myData.pCharList; c != NULL; c = c->next)
		{
			pChar = c->data;
			glPushMatrix();
			
			if (pMainDock->container.bIsHorizontal)
				glTranslatef (pChar->iCurrentX * fTextScale + pMainDock->container.iWidth/2 + pChar->iWidth/2,
					(myConfig.bTextOnTop ?
						pMainDock->container.iHeight - (myData.iTextHeight - pChar->iHeight/2) :
						pChar->iHeight/2),
					0.);  // aligne en bas.
			else
				glTranslatef ((myConfig.bTextOnTop ?
						pMainDock->container.iHeight - (myData.iTextHeight - pChar->iHeight/2) :
						pChar->iHeight/2),
					pChar->iCurrentX * fTextScale + pMainDock->container.iWidth/2 + pChar->iWidth/2,
					0.);  // aligne en bas.
			
			double fRotationAngle = pChar->fRotationAngle;
			if (myConfig.iAppearanceDuration != 0)
			{
				glBindTexture (GL_TEXTURE_2D, pChar->iTexture);
				
				glScalef (pChar->iWidth * fTextScale, pChar->iHeight * fTextScale, 1.);
				
				glRotatef (fRotationAngle, 1., 0., 0.);
				glRotatef (MIN (fRotationAngle/3, STATIC_ANGLE), 0., 0., 1.);
				glRotatef (MIN (fRotationAngle/3, STATIC_ANGLE), 0., 1., 0.);
				
				glPolygonMode (GL_FRONT, GL_FILL);
				double a = .5 / sqrt (2);
				glBegin(GL_QUADS);
				// Front Face (note that the texture's corners have to match the quad's corners)
				glNormal3f(0,0,1);
				glTexCoord2f (0., 0.); glVertex3f(-a,  a,  a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (1., 0.); glVertex3f( a,  a,  a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.); glVertex3f( a, -a,  a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.); glVertex3f(-a, -a,  a);  // Top Left Of The Texture and Quad
				// Back Face
				glNormal3f(0,0,-1);
				glTexCoord2f (1., 0.); glVertex3f( -a, a, -a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.); glVertex3f( -a, -a, -a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.); glVertex3f(a, -a, -a);  // Top Left Of The Texture and Quad
				glTexCoord2f (0., 0.); glVertex3f(a, a, -a);  // Bottom Left Of The Texture and Quad
				// Top Face
				glNormal3f(0,1,0);
				glTexCoord2f (0., 1.); glVertex3f(-a,  a,  a);  // Top Left Of The Texture and Quad
				glTexCoord2f (0., 0.); glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (1., 0.); glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.); glVertex3f( a,  a,  a);  // Top Right Of The Texture and Quad
				// Bottom Face
				glNormal3f(0,-1,0);
				glTexCoord2f (1., 1.); glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.); glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
				glTexCoord2f (0., 0.); glVertex3f(-a, -a,  a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (1., 0.); glVertex3f( a, -a,  a);  // Bottom Right Of The Texture and Quad
				// Right face
				glNormal3f(1,0,0);
				glTexCoord2f (1., 0.);  glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.);  glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.);  glVertex3f( a, -a,  a);  // Top Left Of The Texture and Quad
				glTexCoord2f (0., 0.);  glVertex3f( a,  a,  a);  // Bottom Left Of The Texture and Quad
				// Left Face
				glNormal3f(-1,0,0);
				glTexCoord2f (0., 0.);  glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
				glTexCoord2f (1., 0.);  glVertex3f(-a,  a,  a);  // Bottom Right Of The Texture and Quad
				glTexCoord2f (1., 1.);  glVertex3f(-a, -a,  a);  // Top Right Of The Texture and Quad
				glTexCoord2f (0., 1.);  glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
				glEnd();
			}
			else
			{
				_cairo_dock_apply_texture_at_size (pChar->iTexture, pChar->iWidth, pChar->iHeight);
			}
			
			glPopMatrix();
		}
		gldi_gl_container_set_ortho_view (CAIRO_CONTAINER (pMainDock));
		if (pMainDock->container.bIsHorizontal)
		{
			glTranslatef (-pMainDock->container.iWidth/2, -pMainDock->container.iHeight/2, 0.);
		}
		else
		{
			glTranslatef (-pMainDock->container.iHeight/2, -pMainDock->container.iWidth/2, 0.);
		}
		glDisable (GL_DEPTH_TEST);
	}
}
