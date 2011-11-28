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
#include <GL/gl.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "bilinear-gradation-texture.h"


gboolean cd_drop_indicator_render (gpointer pUserData, CairoDock *pDock, cairo_t *pCairoContext)
{
	CDDropIndicatorData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pCairoContext != NULL)
	{
		if (pData->fAlpha > 0)
		{
			cairo_save (pCairoContext);
			double fX = pDock->container.iMouseX - myData.dropIndicator.iWidth / 2;
			if (pDock->container.bIsHorizontal)
				cairo_rectangle (pCairoContext,
					(int) pDock->container.iMouseX - myData.dropIndicator.iWidth/2,
					(int) (pDock->container.bDirectionUp ? 0 : pDock->iActiveHeight - 2*myData.dropIndicator.iHeight),
					(int) myData.dropIndicator.iWidth,
					(int) (pDock->container.bDirectionUp ? 2*myData.dropIndicator.iHeight : pDock->iActiveHeight));
			else
				cairo_rectangle (pCairoContext,
					(int) (pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->iActiveHeight : pDock->iActiveHeight - 2*myData.dropIndicator.iHeight),
					(int) pDock->container.iMouseX - myData.dropIndicator.iWidth/2,
					(int) (pDock->container.bDirectionUp ? 2*myData.dropIndicator.iHeight : pDock->iActiveHeight),
					(int) myData.dropIndicator.iWidth);
			cairo_clip (pCairoContext);
			
			//cairo_move_to (pCairoContext, fX, 0);
			if (pDock->container.bIsHorizontal)
				cairo_translate (pCairoContext, fX, (pDock->container.bDirectionUp ? 0 : pDock->iActiveHeight));
			else
				cairo_translate (pCairoContext, (pDock->container.bDirectionUp ? 0 : pDock->iActiveHeight), fX);
			double fRotationAngle = (pDock->container.bIsHorizontal ? (pDock->container.bDirectionUp ? 0 : G_PI) : (pDock->container.bDirectionUp ? -G_PI/2 : G_PI/2));
			cairo_rotate (pCairoContext, fRotationAngle);
			
			cairo_translate (pCairoContext, 0, pData->iDropIndicatorOffset);
			cairo_pattern_t* pPattern = cairo_pattern_create_for_surface (myData.dropIndicator.pSurface);
			g_return_val_if_fail (cairo_pattern_status (pPattern) == CAIRO_STATUS_SUCCESS, CAIRO_DOCK_LET_PASS_NOTIFICATION);
			cairo_pattern_set_extend (pPattern, CAIRO_EXTEND_REPEAT);
			cairo_set_source (pCairoContext, pPattern);
			
			cairo_translate (pCairoContext, 0, - pData->iDropIndicatorOffset);
			cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
				0.,
				0.,
				2*myData.dropIndicator.iHeight);  // de haut en bas.
			g_return_val_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS, CAIRO_DOCK_LET_PASS_NOTIFICATION);
		
			cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
			cairo_pattern_add_color_stop_rgba (pGradationPattern,
				0.,
				0.,
				0.,
				0.,
				0.);
			cairo_pattern_add_color_stop_rgba (pGradationPattern,
				0.4,
				0.,
				0.,
				0.,
				pData->fAlpha);
			cairo_pattern_add_color_stop_rgba (pGradationPattern,
				0.5,
				0.,
				0.,
				0.,
				pData->fAlpha);
			cairo_pattern_add_color_stop_rgba (pGradationPattern,
				1.,
				0.,
				0.,
				0.,
				0.);
		
			cairo_mask (pCairoContext, pGradationPattern);
			//cairo_paint (pCairoContext);
			
			cairo_pattern_destroy (pPattern);
			cairo_pattern_destroy (pGradationPattern);
			cairo_restore (pCairoContext);
		}
		
		if (pData->fAlphaHover > 0 && myData.hoverIndicator.pSurface != NULL)
		{
			Icon *pIcon = cairo_dock_get_pointed_icon (pDock->icons);
			if (pIcon != NULL && ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			{
				cairo_save (pCairoContext);
				if (pDock->container.bIsHorizontal)
					cairo_translate (pCairoContext,
						pIcon->fDrawX + 2./3*pIcon->fWidth*pIcon->fScale,
						pIcon->fDrawY);
				else
					cairo_translate (pCairoContext,
						pIcon->fDrawY + 2./3*pIcon->fWidth*pIcon->fScale,
						pIcon->fDrawX);
				cairo_set_source_surface (pCairoContext, myData.hoverIndicator.pSurface, 0., 0.);
				cairo_paint_with_alpha (pCairoContext, pData->fAlphaHover);
				cairo_restore (pCairoContext);
			}
		}
	}
	else
	{
		if (pData->fAlpha > 0)
		{
			;double fX = pDock->container.iMouseX;
			double fY = (pDock->container.bDirectionUp ? pDock->iActiveHeight - myData.dropIndicator.iHeight : myData.dropIndicator.iHeight);
			glPushMatrix();
			glLoadIdentity();
			
			if (pDock->container.bIsHorizontal)
			{
				fX = pDock->container.iMouseX;
				fY = (pDock->container.bDirectionUp ? pDock->iActiveHeight - myData.dropIndicator.iHeight : myData.dropIndicator.iHeight);
				glTranslatef (fX, fY, - myData.dropIndicator.iWidth-1.);
				if (! pDock->container.bDirectionUp)
					glScalef (1., -1., 1.);
			}
			else
			{
				fX = pDock->container.iWidth - pDock->container.iMouseX;
				fY = (! pDock->container.bDirectionUp ? pDock->iActiveHeight - myData.dropIndicator.iHeight : pDock->container.iHeight - pDock->iActiveHeight + myData.dropIndicator.iHeight);
				glTranslatef (fY, fX, - myData.dropIndicator.iWidth-1.);
				glRotatef ((pDock->container.bDirectionUp ? 90. : -90.), 0., 0., 1.);
			}
			
			glRotatef (pData->iDropIndicatorRotation, 0., 1., 0.);
			
			//\_________________ On decale la texture vers le bas.
			glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
			glPushMatrix();
			glLoadIdentity(); // On la reset
			glTranslatef(.0, - (double)pData->iDropIndicatorOffset / myData.dropIndicator.iHeight, 0.);
			glScalef (1., -2., 1.);
			glMatrixMode(GL_MODELVIEW); // On revient sur la matrice d'affichage
			
			//\_________________ On dessine l'indicateur.
			glEnable (GL_BLEND);
			if (pData->fAlpha != 1)
				_cairo_dock_set_blend_alpha ();
			else
				_cairo_dock_set_blend_over();
			
			//glEnable(GL_DEPTH_TEST);
			glScalef (myData.dropIndicator.iWidth, myData.dropIndicator.iHeight, myData.dropIndicator.iWidth);
			glColor4f(1.0f, 1.0f, 1.0f, pData->fAlpha);
			glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
			
			glEnable(GL_TEXTURE);
			glActiveTextureARB(GL_TEXTURE0_ARB); // Go pour le multitexturing 1ere passe
			glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
			glBindTexture(GL_TEXTURE_2D, myData.dropIndicator.iTexture);
			glActiveTextureARB(GL_TEXTURE1_ARB); // Go pour le texturing 2eme passe
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, myData.iBilinearGradationTexture);
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Le mode de combinaison des textures
			///glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);  // multiplier les alpha.
			//glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			
			glBegin(GL_QUADS);
			glNormal3f(0,0,1);
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB,0., 0.); glMultiTexCoord2fARB( GL_TEXTURE1_ARB,0., 0.); glVertex3f(-0.5, -1., 0.);  // Bottom Left Of The Texture and Quad
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB,1., 0.); glMultiTexCoord2fARB( GL_TEXTURE1_ARB,1., 0.); glVertex3f( 0.5, -1., 0.);  // Bottom Right Of The Texture and Quad
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB,1., 1.); glMultiTexCoord2fARB( GL_TEXTURE1_ARB,1., 1.); glVertex3f( 0.5, 1., 0.);  // Top Right Of The Texture and Quad
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB,0., 1.); glMultiTexCoord2fARB( GL_TEXTURE1_ARB,0., 1.); glVertex3f(-0.5, 1., 0.);  // Top Left Of The Texture and Quad
			glNormal3f(1,0,0);
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB,0., 0.); glMultiTexCoord2fARB( GL_TEXTURE1_ARB,0., 0.); glVertex3f(0., -1., -0.5);  // Bottom Left Of The Texture and Quad
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB,1., 0.); glMultiTexCoord2fARB( GL_TEXTURE1_ARB,1., 0.); glVertex3f(0., -1.,  0.5);  // Bottom Right Of The Texture and Quad
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB,1., 1.); glMultiTexCoord2fARB( GL_TEXTURE1_ARB,1., 1.); glVertex3f(0.,  1.,  0.5);  // Top Right Of The Texture and Quad
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB,0., 1.); glMultiTexCoord2fARB( GL_TEXTURE1_ARB,0., 1.); glVertex3f(0.,  1., -0.5);  // Top Left Of The Texture and Quad
			glEnd();
			
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable (GL_BLEND);
			_cairo_dock_set_blend_alpha ();
			glPopMatrix();
			
			//\_________________ On remet la matrice des textures.
			glMatrixMode(GL_TEXTURE);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}
		
		if (pData->fAlphaHover > 0 && myData.hoverIndicator.iTexture != 0)
		{
			Icon *pIcon = cairo_dock_get_pointed_icon (pDock->icons);
			if (pIcon != NULL && ! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			{
				_cairo_dock_enable_texture ();
				_cairo_dock_set_blend_over ();
				glPushMatrix ();
				if (pDock->container.bIsHorizontal)
					glTranslatef (pIcon->fDrawX + 5./6*pIcon->fWidth*pIcon->fScale,
						pDock->iActiveHeight - pIcon->fDrawY - 1./6*pIcon->fHeight*pIcon->fScale,
						0.);
				else
					glTranslatef (pIcon->fDrawY + 5./6*pIcon->fHeight*pIcon->fScale,
						pDock->container.iWidth - (pIcon->fDrawX + 1./6*pIcon->fWidth*pIcon->fScale),
						0.);
				_cairo_dock_apply_texture_at_size_with_alpha (myData.hoverIndicator.iTexture,
					myData.hoverIndicator.iWidth,
					myData.hoverIndicator.iHeight,
					pData->fAlphaHover);
				glPopMatrix ();
				_cairo_dock_disable_texture ();
			}
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_drop_indicator_mouse_moved (gpointer pUserData, CairoDock *pDock, gboolean *bStartAnimation)
{
	CDDropIndicatorData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	
	if (pDock->bIsDragging)
	{
		if (pData == NULL)
		{
			pData = g_new0 (CDDropIndicatorData, 1);
			CD_APPLET_SET_MY_DOCK_DATA (pDock, pData);
		}
		if (pDock->bCanDrop)
			pData->fAlpha = 1.;
		else
			pData->fAlphaHover = 1.;
	}
	else if (pData != NULL && pData->fAlpha <= 0 && pData->fAlphaHover <= 0)
	{
		g_free (pData);
		pData = NULL;
		CD_APPLET_SET_MY_DOCK_DATA (pDock, NULL);
	}
	
	if (pData != NULL)
		*bStartAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

#define delta_alpha 0.06
gboolean cd_drop_indicator_update_dock (gpointer pUserData, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDDropIndicatorData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	pData->iDropIndicatorOffset += myConfig.iSpeed;
	if (pData->iDropIndicatorOffset > 2*myData.dropIndicator.iHeight)
		pData->iDropIndicatorOffset -= 2*myData.dropIndicator.iHeight;
        
	double dt = cairo_dock_get_animation_delta_t (CAIRO_CONTAINER (pDock));
	pData->iDropIndicatorRotation += myConfig.fRotationSpeed * 360. * dt/1e3;
	
	if (pDock->bCanDrop)
	{
		pData->fAlphaHover -= delta_alpha;
		*bContinueAnimation = TRUE;
	}
	else
	{
		pData->fAlpha -= delta_alpha;
		if (!pDock->bIsDragging)
			pData->fAlphaHover -= delta_alpha;
		
		if (pData->fAlpha <= 0 && pData->fAlphaHover <= 0)
		{
			g_free (pData);
			pData = NULL;
			CD_APPLET_SET_MY_DOCK_DATA (pDock, NULL);
		}
		else
			*bContinueAnimation = TRUE;
	}
	
	GdkRectangle rect = {(int) pDock->container.iMouseX - myData.dropIndicator.iWidth/2,
		(int) (pDock->container.bDirectionUp ? 0 : pDock->iActiveHeight - 2*myData.dropIndicator.iHeight),
		(int) myData.dropIndicator.iWidth,
		(int) 2*myData.dropIndicator.iHeight};
	if (! pDock->container.bIsHorizontal)
	{
		rect.x = (int) (pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->iActiveHeight : pDock->iActiveHeight - 2*myData.dropIndicator.iHeight);
		rect.y = (int) pDock->container.iMouseX - myData.dropIndicator.iWidth/2;
		rect.width = (int) 2*myData.dropIndicator.iHeight;
		rect.height = (int) myData.dropIndicator.iWidth;
	}
	//g_print ("rect (%d;%d) (%dx%d)\n", rect.x, rect.y, rect.width, rect.height);
	if (rect.width > 0 && rect.height > 0)
	{
		cairo_dock_redraw_container_area (CAIRO_CONTAINER (pDock), &rect);
	}
	
	if (pData && pData->fAlphaHover > 0)
	{
		Icon *pIcon = cairo_dock_get_pointed_icon (pDock->icons);
		if (pIcon != NULL)
			cairo_dock_redraw_icon (pIcon, CAIRO_CONTAINER (pDock));
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


void cd_drop_indicator_load_drop_indicator (gchar *cImage, int iWidth, int iHeight)
{
	cd_message ("%s (%s)", __func__, cImage);
	cairo_dock_load_image_buffer (&myData.dropIndicator,
		cImage,
		iWidth,
		iHeight,
		CAIRO_DOCK_KEEP_RATIO);
	if (myData.dropIndicator.pSurface == NULL)  // image inexistante ou illisible.
	{
		cairo_dock_load_image_buffer (&myData.dropIndicator,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_DEFAULT_DROP_INDICATOR_NAME,
			iWidth,
			iHeight,
			CAIRO_DOCK_KEEP_RATIO);
	}
	if (myData.dropIndicator.iTexture != 0 && myData.iBilinearGradationTexture == 0)
	{
		myData.iBilinearGradationTexture = cairo_dock_load_texture_from_raw_data (gradationTex, 1, 32);
	}
}

void cd_drop_indicator_load_hover_indicator (gchar *cImage, int iWidth, int iHeight)
{
	cd_message ("%s (%s)", __func__, cImage);
	cairo_dock_load_image_buffer (&myData.hoverIndicator,
		cImage,
		iWidth,
		iHeight,
		CAIRO_DOCK_KEEP_RATIO);
	if (myData.hoverIndicator.pSurface == NULL)  // image inexistante ou illisible.
	{
		cairo_dock_load_image_buffer (&myData.hoverIndicator,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_DEFAULT_HOVER_INDICATOR_NAME,
			iWidth,
			iHeight,
			CAIRO_DOCK_KEEP_RATIO);
	}
}

void cd_drop_indicator_free_buffers (void)
{
	cairo_dock_unload_image_buffer (&myData.dropIndicator);
	cairo_dock_unload_image_buffer (&myData.hoverIndicator);
}


gboolean cd_drop_indicator_stop_dock (gpointer data, CairoDock *pDock)
{
	CDDropIndicatorData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData != NULL)
	{
		g_free (pData);
		CD_APPLET_SET_MY_DOCK_DATA (pDock, NULL);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
