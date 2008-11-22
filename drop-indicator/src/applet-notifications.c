/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <GL/gl.h> 
#include <gdk/x11/gdkglx.h>
#include <gtk/gtkgl.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "bilinear-gradation-texture.h"

gboolean cd_drop_indicator_render (gpointer pUserData, CairoDock *pDock)
{
	CDDropIndicatorData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	double fX = pDock->iMouseX;
	double fY = (pDock->bDirectionUp ? pDock->iCurrentHeight - myData.fDropIndicatorHeight : myData.fDropIndicatorHeight);
	glPushMatrix();
	glLoadIdentity();
	
	if (pDock->bHorizontalDock)
		glTranslatef (fX, fY, - myData.fDropIndicatorWidth-1.);
	else
		glTranslatef (fY, fX, - myData.fDropIndicatorWidth-1.);
	double fRotationAngle = (pDock->bHorizontalDock ? (pDock->bDirectionUp ? 0 : 180.) : (pDock->bDirectionUp ? -90. : 90.));
	glRotatef (fRotationAngle, 0., 0., 1.);
	
	glRotatef (pData->iDropIndicatorRotation, 0., 1., 0.);
	
	//\_________________ On decale la texture vers le bas.
	glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
	glPushMatrix();
	glLoadIdentity(); // On la reset
	glTranslatef(.0, - pData->iDropIndicatorOffset / myData.fDropIndicatorHeight, 0.);
	glScalef (1., -2., 1.);
	glMatrixMode(GL_MODELVIEW); // On revient sur la matrice d'affichage
	
	//\_________________ On dessine l'indicateur.
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//glEnable(GL_DEPTH_TEST);
	glScalef (myData.fDropIndicatorWidth, myData.fDropIndicatorHeight, myData.fDropIndicatorWidth);
	glColor4f(1.0f, 1.0f, 1.0f, pData->fAlpha);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	
	glEnable(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE0_ARB); // Go pour le multitexturing 1ere passe
	glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
	glBindTexture(GL_TEXTURE_2D, myData.iDropIndicatorTexture);
	glActiveTextureARB(GL_TEXTURE1_ARB); // Go pour le texturing 2eme passe
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, myData.iBilinearGradationTexture);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Le mode de combinaison des textures
	glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);  // multiplier les alpha.
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
	
	//\_________________ On remet la matrice des textures.
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	
	
	glEnable(GL_TEXTURE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, myData.iDropIndicatorTexture);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//\_________________ On decale la texture vers le bas.
	glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
	glPushMatrix();
	glLoadIdentity(); // On la reset
	glTranslatef(.0, - pData->iDropIndicatorOffset / myData.fDropIndicatorHeight, 0.);
	glScalef (1., -2., 1.);
	glMatrixMode(GL_MODELVIEW); // On revient sur la matrice d'affichage
	
	//\_________________ On dessine l'indicateur.
	glEnable(GL_DEPTH_TEST);
	glScalef (myData.fDropIndicatorWidth, myData.fDropIndicatorHeight, myData.fDropIndicatorWidth);
	glColor4f(1.0f, 1.0f, 1.0f, pData->fAlpha);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	
	glBegin(GL_QUADS);
	glNormal3f(0,0,1);
	glTexCoord2f(0., 0.); glVertex3f(-0.5, -1., 0.);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1., 0.); glVertex3f( 0.5, -1., 0.);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1., 1.); glVertex3f( 0.5,  1., 0.);  // Top Right Of The Texture and Quad
	glTexCoord2f(0., 1.); glVertex3f(-0.5,  1., 0.);  // Top Left Of The Texture and Quad
	glNormal3f(1,0,0);
	glTexCoord2f(0., 0.); glVertex3f(0., -1., -0.5);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1., 0.); glVertex3f(0., -1.,  0.5);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1., 1.); glVertex3f(0.,  1.,  0.5);  // Top Right Of The Texture and Quad
	glTexCoord2f(0., 1.); glVertex3f(0.,  1., -0.5);  // Top Left Of The Texture and Quad
	glEnd();
	glPopMatrix();

	//\_________________ On decale la matrice des textures pour centrer le masque.
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(.0, .0, 0.);
	glMatrixMode(GL_MODELVIEW);
	
	glBindTexture (GL_TEXTURE_2D, myData.iBilinearGradationTexture);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	
	//\_________________ On dessine le masque.
	if (pDock->bHorizontalDock)
		glTranslatef (fX, fY, -1.);  // on se place devant.
	else
		glTranslatef (fY, fX, -1.);
	glDisable(GL_DEPTH_TEST);
	glScalef (myData.fDropIndicatorWidth, myData.fDropIndicatorHeight, 1.);
	glColor4f(1.0f, 1.0f, 1.0f, 1.);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0., 0.); glVertex3f(-0.5,  1., 1.);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1., 0.); glVertex3f( 0.5,  1., 1.);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1., 1.); glVertex3f( 0.5, -1., 1.);  // Top Right Of The Texture and Quad
	glTexCoord2f(0., 1.); glVertex3f(-0.5, -1., 1.);  // Top Left Of The Texture and Quad
	glEnd();
	
	//\_________________ On remet la matrice des textures.
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	glDisable(GL_TEXTURE_2D);
	glDisable (GL_BLEND);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_drop_indicator_mouse_moved (gpointer pUserData, CairoDock *pDock, gboolean *bStartAnimation)
{
	CDDropIndicatorData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	
	if (pDock->bCanDrop)
	{
		if (pData == NULL)
		{
			pData = g_new0 (CDDropIndicatorData, 1);
			CD_APPLET_SET_MY_DOCK_DATA (pDock, pData);
		}
		pData->fAlpha = 1.;
	}
	else
	{
		if (pData != NULL)
		{
			if (pData->fAlpha <= 0)
			{
				g_free (pData);
				pData = NULL;
				CD_APPLET_SET_MY_DOCK_DATA (pDock, NULL);
			}
		}
	}
	
	*bStartAnimation = (pData != NULL);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_drop_indicator_update_dock (gpointer pUserData, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDDropIndicatorData *pData = CD_APPLET_GET_MY_DOCK_DATA (pDock);
	if (pData == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	pData->iDropIndicatorOffset += myConfig.iSpeed;
	if (pData->iDropIndicatorOffset > 2*myData.fDropIndicatorHeight)
		pData->iDropIndicatorOffset -= 2*myData.fDropIndicatorHeight;
	pData->iDropIndicatorRotation += myConfig.iRotationSpeed;
	if (pDock->bCanDrop)
	{
		*bContinueAnimation = TRUE;
	}
	else
	{
		pData->fAlpha -= .05;
		if (pData->fAlpha <= 0)
		{
			g_free (pData);
			CD_APPLET_SET_MY_DOCK_DATA (pDock, NULL);
		}
		else
			*bContinueAnimation = TRUE;
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


void cd_drop_indicator_load_drop_indicator (gchar *cImagePath, cairo_t* pSourceContext, int iWidth, int iHeight)
{
	g_print ("%s (%s)\n", __func__, cImagePath);
	if (myData.pDropIndicatorSurface != NULL)
		cairo_surface_destroy (myData.pDropIndicatorSurface);
	if (myData.iDropIndicatorTexture != 0)
	{
		glDeleteTextures (1, &myData.iDropIndicatorTexture);
		myData.iDropIndicatorTexture = 0;
	}
	myData.pDropIndicatorSurface = cairo_dock_create_surface_from_image (cImagePath,
		pSourceContext,
		1.,
		iWidth,
		iHeight,
		CAIRO_DOCK_KEEP_RATIO,
		&myData.fDropIndicatorWidth, &myData.fDropIndicatorHeight,
		NULL, NULL);
	if (g_bUseOpenGL && myData.pDropIndicatorSurface != NULL)
	{
		GdkGLContext* pGlContext = gtk_widget_get_gl_context (g_pMainDock->pWidget);
		GdkGLDrawable* pGlDrawable = gtk_widget_get_gl_drawable (g_pMainDock->pWidget);
		if (!gdk_gl_drawable_gl_begin (pGlDrawable, pGlContext))
			return ;
		myData.iDropIndicatorTexture = cairo_dock_create_texture_from_surface (myData.pDropIndicatorSurface);
		
		double fWidth=0, fHeight=0;
		
		gchar *cGradationTexturePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_MASK_INDICATOR_NAME);
		myData.iBilinearGradationTexture = cairo_dock_create_texture_from_image (cGradationTexturePath);
		g_free (cGradationTexturePath);
		myData.iBilinearGradationTexture = cairo_dock_load_texture_from_raw_data (gradationTex, 1, 32);
		
		gdk_gl_drawable_gl_end (pGlDrawable);
	}
}


