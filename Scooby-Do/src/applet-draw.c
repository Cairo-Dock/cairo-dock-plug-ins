/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-draw.h"


void cd_do_render_cairo (CairoDock *pMainDock, cairo_t *pCairoContext)
{
	double fAlpha;
	if (myData.iCloseTime != 0) // animation de fin
		fAlpha = (double) myData.iCloseTime / myConfig.iCloseDuration;
	else
		fAlpha = 1.;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pMainDock->iMagnitudeIndex);  /// a utiliser ...
	
	// dessin du fond.
	double fRadius = myBackground.iDockRadius * myConfig.fFontSizeRatio;
	double fLineWidth = 0.;
	double fFrameWidth = myData.iTextWidth;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	fDockOffsetX = (pMainDock->iCurrentWidth - myData.iTextWidth) / 2;
	fDockOffsetY = (pMainDock->iCurrentHeight - myData.iTextHeight) / 2;
	
	cairo_save (pCairoContext);
	double fDeltaXTrapeze = cairo_dock_draw_frame (pCairoContext, fRadius, fLineWidth, fFrameWidth, myData.iTextHeight, fDockOffsetX, fDockOffsetY, 1, 0., pMainDock->bHorizontalDock);
	cairo_set_line_width (pCairoContext, fLineWidth);
	cairo_set_source_rgba (pCairoContext, myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myConfig.pBackgroundColor[3]);
	cairo_fill (pCairoContext);
	
	cairo_restore (pCairoContext);
	
	// dessin des lettres.
	int x = fDockOffsetX, y = fDockOffsetY;
	CDChar *pChar;
	GList *c;
	for (c = myData.pCharList; c != NULL; c = c->next)
	{
		pChar = c->data;
		cairo_save (pCairoContext);
		
		cairo_translate (pCairoContext,
			x,
			y + (myData.iTextHeight - pChar->iHeight));  // aligne en bas.
		cairo_set_source_surface (pCairoContext, pChar->pSurface, 0., 0.);
		cairo_paint_with_alpha (pCairoContext, fAlpha);
		
		cairo_restore (pCairoContext);
		x += pChar->iWidth;
	}
}

void cd_do_render_opengl (CairoDock *pMainDock)
{
	double fAlpha;
	if (myData.iCloseTime != 0) // animation de fin
		fAlpha = (double) myData.iCloseTime / myConfig.iCloseDuration;
	else
		fAlpha = 1.;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pMainDock->iMagnitudeIndex);  /// a utiliser ...
	double fScale = (1. + fDockMagnitude * g_fAmplitude) / (1 + g_fAmplitude);
	// dessin du fond.
	double fRadius = myBackground.iDockRadius * myConfig.fFontSizeRatio;
	double fLineWidth = 0.;
	double fFrameWidth = myData.iTextWidth;
	double fFrameHeight = myData.iTextHeight * fScale;
	double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
	fDockOffsetX = pMainDock->iCurrentWidth/2 - myData.iTextWidth / 2 * fScale;
	fDockOffsetY = pMainDock->iCurrentHeight/2 + myData.iTextHeight / 2 * fScale;
	
	int iNbVertex = 0;
	const GLfloat *pVertexTab = cairo_dock_generate_rectangle_path (fFrameWidth, fFrameHeight, fRadius, TRUE, &iNbVertex);
	
	glPushMatrix ();
	glEnable(GL_BLEND);
	_cairo_dock_set_blend_alpha ();
	glColor4f (myConfig.pBackgroundColor[0], myConfig.pBackgroundColor[1], myConfig.pBackgroundColor[2], myConfig.pBackgroundColor[3] * fAlpha);
	cairo_dock_draw_frame_background_opengl (0, fFrameWidth+2*fRadius, fFrameHeight, fDockOffsetX, fDockOffsetY, pVertexTab, iNbVertex, pMainDock->bHorizontalDock, pMainDock->bDirectionUp, 0.);
	glPopMatrix();
	
	// dessin des lettres.
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (fAlpha);
	
	//int x = fDockOffsetX, y = fDockOffsetY;
	CDChar *pChar;
	GList *c;
	for (c = myData.pCharList; c != NULL; c = c->next)
	{
		pChar = c->data;
		glPushMatrix();
		
		glTranslatef (pChar->iCurrentX + pMainDock->iCurrentWidth/2 + pChar->iWidth/2,
			pChar->iCurrentY + pMainDock->iCurrentHeight/2 - (myData.iTextHeight - pChar->iHeight)/2 * fScale,
			0.);  // aligne en bas.
		glScalef (1., fScale, 1.);
		_cairo_dock_apply_texture_at_size (pChar->iTexture, pChar->iWidth, pChar->iHeight);
		
		glPopMatrix();
	}
	
	_cairo_dock_disable_texture ();
}



void cd_do_load_pending_caracters (void)
{
	cairo_surface_t *pSurface;
	GLuint iTexture;
	gboolean bLoadTexture = (CAIRO_CONTAINER_IS_OPENGL (g_pMainDock));
	gchar c[2] = {'\0', '\0'};
	double fTextXOffset, fTextYOffset;
	CDChar *pChar;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	int i, iOffsetX=0;
	for (i = myData.iNbValidCaracters-1; i < myData.sCurrentText->len; i++)
	{
		g_print (" on charge la lettre '%c' (%d) tex:%d\n", myData.sCurrentText->str[i], i, bLoadTexture);
		c[0] = myData.sCurrentText->str[i];
		
		pChar = g_new0 (CDChar, 1);
		pChar->c = c[0];
		pChar->iInitialX = g_pMainDock->iMaxDockWidth/2 + iOffsetX;  // il part du coin haut droit.
		pChar->iCurrentX = pChar->iInitialX;
		g_print (" on commence a x=%d\n", pChar->iInitialX);
		myData.pCharList = g_list_append (myData.pCharList, pChar);
		
		// on cree la surface.
		pSurface = cairo_dock_create_surface_from_text (c, pCairoContext, &myConfig.labelDescription, 1., &pChar->iWidth, &pChar->iHeight, &fTextXOffset, &fTextYOffset);
		if (g_pMainDock->bHorizontalDock)
		{
			myData.iTextWidth += pChar->iWidth;
			iOffsetX += pChar->iWidth;
			pChar->iInitialY = g_pMainDock->iMaxDockHeight - pChar->iHeight;
			myData.iTextHeight = MAX (myData.iTextHeight, pChar->iHeight);
		}
		else
		{
			myData.iTextHeight += pChar->iHeight;
			iOffsetX += pChar->iHeight;
			pChar->iInitialY = g_pMainDock->iMaxDockHeight - pChar->iWidth;
			myData.iTextWidth = MAX (myData.iTextWidth, pChar->iWidth);
		}
		
		// on cree la texture.
		if (bLoadTexture)
		{
			pChar->iTexture = cairo_dock_create_texture_from_surface (pSurface);
			cairo_surface_destroy (pSurface);
		}
		else
		{
			pChar->pSurface = pSurface;
		}
	}
	cairo_destroy (pCairoContext);
}

void cd_do_free_char (CDChar *pChar)
{
	if (pChar == NULL)
		return ;
	if (pChar->pSurface != NULL)
	{
		cairo_surface_destroy (pChar->pSurface);
	}
	if (pChar->iTexture != 0)
	{
		_cairo_dock_delete_texture (pChar->iTexture);
	}
	g_free (pChar);
}

void cd_do_free_char_list (GList *pCharList)
{
	g_list_foreach (pCharList, (GFunc) cd_do_free_char, NULL);
	g_list_free (pCharList);
}


void cd_do_compute_final_coords (void)
{
	int x = - myData.iTextWidth / 2;  // par rapport au milieu du dock.
	CDChar *pChar;
	GList *c;
	for (c = myData.pCharList; c != NULL; c = c->next)
	{
		pChar = c->data;
		
		pChar->iFinalX = x;
		pChar->iFinalY = 0;
		x += pChar->iWidth;
		
		pChar->iInitialX = pChar->iCurrentX;
		pChar->iInitialY = pChar->iCurrentY;
	}
}

void cd_do_compute_current_coords (void)
{
	double f = (double) myData.iAppearanceTime / myConfig.iAppearanceDuration;
	int x = - myData.iTextWidth / 2;  // par rapport au milieu du dock.
	CDChar *pChar;
	GList *c;
	for (c = myData.pCharList; c != NULL; c = c->next)
	{
		pChar = c->data;
		pChar->iCurrentX = f * pChar->iInitialX + (1-f) * pChar->iFinalX;
		pChar->iCurrentY = f * pChar->iInitialY + (1-f) * pChar->iFinalY;
	}
}

void cd_do_launch_appearance_animation (void)
{
	cd_do_compute_final_coords ();
	myData.iAppearanceTime = myConfig.iAppearanceDuration;
	cairo_dock_launch_animation (CAIRO_CONTAINER (g_pMainDock));  // animation de disparition.	
}
