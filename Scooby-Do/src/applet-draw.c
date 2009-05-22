/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-session.h"
#include "applet-draw.h"

#define STATIC_ANGLE 15.

void cd_do_render_cairo (CairoDock *pMainDock, cairo_t *pCairoContext)
{
	double fAlpha;
	if (myData.iCloseTime != 0) // animation de fin
		fAlpha = (double) myData.iCloseTime / myConfig.iCloseDuration;
	else
		fAlpha = 1.;
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pMainDock->iMagnitudeIndex);
	double fScale = (1. + fDockMagnitude * g_fAmplitude) / (1 + g_fAmplitude);
	
	if (cd_do_session_is_in_navigation_mode ())  // aucune lettre de tapee, on est en mode navigation => on montre le prompt.
	{
		if (myData.pPromptSurface != NULL)
		{
			double fFrameWidth = myData.iPromptWidth * fScale;
			double fFrameHeight = myData.iPromptHeight * fScale;
			
			double fRelativePositionOffset = myConfig.fRelativePosition * (pMainDock->iCurrentHeight - fFrameHeight) / 2;
			
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
			fDockOffsetX = (pMainDock->iCurrentWidth - fFrameWidth) / 2;
			fDockOffsetY = (pMainDock->iCurrentHeight - fFrameHeight) / 2 + fRelativePositionOffset;
			
			int n = 40;
			fAlpha *= sqrt (fabs ((double) (((myData.iPromptAnimationCount + n) % (2*n)) - n) / n));
			
			cairo_set_source_surface (pCairoContext, myData.pPromptSurface, fDockOffsetX, fDockOffsetY);
			if (fScale != 1)
				cairo_scale (pCairoContext, fScale, fScale);
			cairo_paint_with_alpha (pCairoContext, fAlpha);
		}
	}
	else  // si des icones sont selectionnees, on les dessine.
	{
		double fFrameWidth = myData.iTextWidth * fScale;
		double fFrameHeight = myData.iTextHeight * fScale;
		
		double fRelativePositionOffset = myConfig.fRelativePosition * (pMainDock->iCurrentHeight - fFrameHeight) / 2;

		// dessin du fond du texte.
		double fRadius = myBackground.iDockRadius * myConfig.fFontSizeRatio;
		double fLineWidth = 0.;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = (pMainDock->iCurrentWidth - fFrameWidth) / 2;
		fDockOffsetY = (pMainDock->iCurrentHeight - fFrameHeight) / 2 + fRelativePositionOffset;
		
		cairo_save (pCairoContext);
		double fDeltaXTrapeze = cairo_dock_draw_frame (pCairoContext, fRadius, fLineWidth, fFrameWidth, fFrameHeight, fDockOffsetX, fDockOffsetY, 1, 0., pMainDock->bHorizontalDock);
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
				pChar->iCurrentX * fScale + pMainDock->iCurrentWidth/2,
				pChar->iCurrentY + pMainDock->iCurrentHeight/2 + (myData.iTextHeight/2 - pChar->iHeight) * fScale + fRelativePositionOffset);  // aligne en bas.
			cairo_set_source_surface (pCairoContext, pChar->pSurface, 0., 0.);
			if (fScale != 1)
				cairo_scale (pCairoContext, fScale, fScale);
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
	
	double fDockMagnitude = cairo_dock_calculate_magnitude (pMainDock->iMagnitudeIndex);
	double fScale = (1. + fDockMagnitude * g_fAmplitude) / (1 + g_fAmplitude);
	
	if (cd_do_session_is_in_navigation_mode ())  // aucune lettre de tapee, on est en mode navigation => on montre le prompt.
	{
		if (myData.iPromptTexture != 0)
		{
			double fFrameWidth = myData.iPromptWidth * fScale;
			double fFrameHeight = myData.iPromptHeight * fScale;
			
			double fRelativePositionOffset = myConfig.fRelativePosition * (pMainDock->iCurrentHeight - fFrameHeight) / 2;
			
			double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
			fDockOffsetX = (pMainDock->iCurrentWidth - fFrameWidth) / 2;
			fDockOffsetY = (pMainDock->iCurrentHeight - fFrameHeight) / 2 + fRelativePositionOffset;
			
			int n = 40;
			fAlpha *= sqrt (fabs ((double) (((myData.iPromptAnimationCount + n) % (2*n)) - n) / n));
			
			glPushMatrix ();
			glTranslatef (pMainDock->iCurrentWidth/2, pMainDock->iCurrentHeight/2, 0.);
			if (fScale != 1)
				glScalef (fScale, fScale, 1.);
			
			_cairo_dock_enable_texture ();
			_cairo_dock_set_blend_alpha ();
			
			_cairo_dock_apply_texture_at_size_with_alpha (myData.iPromptTexture, fFrameWidth, fFrameHeight, fAlpha);
			
			_cairo_dock_disable_texture ();
			
			glPopMatrix();
		}
	}
	else  // si des icones sont selectionnees, on les dessine.
	{
		double fFrameWidth = myData.iTextWidth * fScale;
		double fFrameHeight = myData.iTextHeight * fScale;
		double fRelativePositionOffset = myConfig.fRelativePosition * (pMainDock->iCurrentHeight - fFrameHeight) / 2;
		
		// dessin du fond.
		double fRadius = myBackground.iDockRadius * myConfig.fFontSizeRatio;
		double fLineWidth = 0.;
		
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = pMainDock->iCurrentWidth/2 - fFrameWidth / 2;
		fDockOffsetY = pMainDock->iCurrentHeight/2 + fFrameHeight / 2 - fRelativePositionOffset;
		
		int iNbVertex = 0;
		const GLfloat *pVertexTab = cairo_dock_generate_rectangle_path (fFrameWidth, fFrameHeight, fRadius, TRUE, &iNbVertex);
		
		glPushMatrix ();
		glEnable(GL_BLEND);
		_cairo_dock_set_blend_alpha ();
		glColor4f (myConfig.pFrameColor[0], myConfig.pFrameColor[1], myConfig.pFrameColor[2], myConfig.pFrameColor[3] * fAlpha);
		cairo_dock_draw_frame_background_opengl (0, fFrameWidth+2*fRadius, fFrameHeight, fDockOffsetX, fDockOffsetY, pVertexTab, iNbVertex, pMainDock->bHorizontalDock, pMainDock->bDirectionUp, 0.);
		glPopMatrix();
		
		// dessin des lettres.
		_cairo_dock_enable_texture ();
		_cairo_dock_set_blend_alpha ();
		_cairo_dock_set_alpha (fAlpha);
		
		cairo_dock_set_perspective_view (pMainDock->iCurrentWidth, pMainDock->iCurrentHeight);
		glTranslatef (-pMainDock->iCurrentWidth/2, -pMainDock->iCurrentHeight/2, 0.);
		glEnable (GL_DEPTH_TEST);
		
		CDChar *pChar;
		GList *c;
		for (c = myData.pCharList; c != NULL; c = c->next)
		{
			pChar = c->data;
			glPushMatrix();
			
			glTranslatef (pChar->iCurrentX * fScale + pMainDock->iCurrentWidth/2 + pChar->iWidth/2,
				- pChar->iCurrentY + pMainDock->iCurrentHeight/2 - (myData.iTextHeight - pChar->iHeight)/2 * fScale - fRelativePositionOffset,
				0.);  // aligne en bas.
			
			double fRotationAngle = pChar->fRotationAngle;
			if (myConfig.iAppearanceDuration != 0)
			{
				glBindTexture (GL_TEXTURE_2D, pChar->iTexture);
				
				glScalef (pChar->iWidth * fScale, fScale * pChar->iHeight, 1.);
				
				glRotatef (fRotationAngle+STATIC_ANGLE, 1., 0., 0.);
				glRotatef (STATIC_ANGLE, 0., 0., 1.);
				glRotatef (STATIC_ANGLE, 0., 1., 0.);
				
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
		cairo_dock_set_ortho_view (pMainDock->iCurrentWidth, pMainDock->iCurrentHeight);
		glTranslatef (-pMainDock->iCurrentWidth/2, -pMainDock->iCurrentHeight/2, 0.);
		glDisable (GL_DEPTH_TEST);
		_cairo_dock_disable_texture ();
	}
}
