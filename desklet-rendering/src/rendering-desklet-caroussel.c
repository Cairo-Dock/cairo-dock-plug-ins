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

#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "rendering-desklet-caroussel.h"

#define CAROUSSEL_RATIO_MAIN_ICON_DESKLET .5

// Pour dessiner une icone, avec son quickinfo, dans la matrice courante.
static void _render_one_icon_and_quickinfo_opengl (Icon *pIcon, CairoContainer *pContainer, gboolean bIsReflect)
{
	if (pIcon == NULL)  // peut arriver avant de lier l'icone au desklet.
		return ;

	if (pIcon->iIconTexture != 0)
	{
		glPushMatrix ();
			cairo_dock_draw_icon_texture (pIcon, pContainer);
			glColor4f(1., 1., 1., 1.);
		glPopMatrix ();
	}
	if (pIcon->iLabelTexture != 0 && !bIsReflect)
	{
		glPushMatrix ();
			glTranslatef (0.,
				(pIcon->fHeight + pIcon->iTextHeight)/2,
				0.);
			cairo_dock_draw_texture (pIcon->iLabelTexture,
				pIcon->iTextWidth,
				pIcon->iTextHeight);
		glPopMatrix ();
	}
	if (pIcon->iQuickInfoTexture != 0 && !bIsReflect)
	{
		glPushMatrix ();
			glTranslatef (0.,
				(- pIcon->fHeight - pIcon->iQuickInfoHeight)/2,
				0.);
			cairo_dock_draw_texture (pIcon->iQuickInfoTexture,
				pIcon->iQuickInfoWidth,
				pIcon->iQuickInfoHeight);
		glPopMatrix ();
	}
}


static inline void _caroussel_rotate_delta(CairoDesklet *pDesklet, double fDeltaTheta)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	pCaroussel->fCurrentRotationSpeed = fDeltaTheta;
	pCaroussel->fRotationAngle += fDeltaTheta;
	if (pCaroussel->fRotationAngle >= 2*G_PI)
		pCaroussel->fRotationAngle -= 2*G_PI;
	else if (pCaroussel->fRotationAngle < 0)
		pCaroussel->fRotationAngle += 2*G_PI;
	gtk_widget_queue_draw (pDesklet->container.pWidget);
}
static gboolean on_update_desklet (gpointer pUserData, CairoDesklet *pDesklet, gboolean *bContinueAnimation)
{
	if (pDesklet->icons != NULL)
	{
		CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
		if (pCaroussel == NULL)
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		
		if (! pDesklet->container.bInside)  // on est en-dehors du desklet, on ralentit.
		{
			_caroussel_rotate_delta (pDesklet, pCaroussel->fCurrentRotationSpeed * .85);
			if (fabs (pCaroussel->fCurrentRotationSpeed) < .5/180.*G_PI)  // vitesse de rotation epsilonesque, on quitte.
			{
				pCaroussel->fCurrentRotationSpeed = 0;
				return CAIRO_DOCK_LET_PASS_NOTIFICATION;
			}
			*bContinueAnimation = TRUE;
		}
		else if (pDesklet->container.iMouseX <= pDesklet->container.iWidth*0.3)  // si on est dans la marge de 30% de la largeur du desklet a gauche, alors on tourne a droite
		{
			// La force de rotation va de 0 (lorsqu'on est a 30%) jusqu'a
			// pCaroussel->fDeltaTheta / 10. (lorsqu'on est a 0%)
			double fDeltaRotation = (pCaroussel->fDeltaTheta / 10) *
			                        (pDesklet->container.iWidth*0.3 - pDesklet->container.iMouseX)/(pDesklet->container.iWidth*0.3);
			_caroussel_rotate_delta( pDesklet, fDeltaRotation );
			*bContinueAnimation = TRUE;
		}
		// si on est dans la marge de 30% de la largeur du desklet a droite,
		// alors on tourne a gauche (-1)
		else if( pDesklet->container.iMouseX >= pDesklet->container.iWidth*0.7 )
		{
			// La force de rotation va de 0 (lorsqu'on est a 70%) jusqu'a
			// pCaroussel->fDeltaTheta / 10. (lorsqu'on est a 100%)
			double fDeltaRotation = - (pCaroussel->fDeltaTheta / 10) *
			                        (pDesklet->container.iMouseX - pDesklet->container.iWidth*0.7)/(pDesklet->container.iWidth*0.3);
			_caroussel_rotate_delta( pDesklet, fDeltaRotation );
			*bContinueAnimation = TRUE;
		}
		else
		{
			pCaroussel->fCurrentRotationSpeed = 0.;
		}
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
static gboolean on_mouse_move (gpointer pUserData, CairoDesklet *pDesklet, gboolean *bStartAnimation)
{
	if (pDesklet->icons != NULL)
	{
		CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
		if (pCaroussel == NULL)
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
		if (pCaroussel->b3D && (pDesklet->container.iMouseX <= pDesklet->container.iWidth*0.3 || pDesklet->container.iMouseX >= pDesklet->container.iWidth*0.7))
			*bStartAnimation = TRUE;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

static CDCarousselParameters *configure (CairoDesklet *pDesklet, gpointer *pConfig)
{
	CDCarousselParameters *pCaroussel = g_new0 (CDCarousselParameters, 1);
	if (pConfig != NULL)
	{
		pCaroussel->b3D = GPOINTER_TO_INT (pConfig[0]);
		pCaroussel->bRotateIconsOnEllipse = GPOINTER_TO_INT (pConfig[1]);
	}
	
	cairo_dock_register_notification_on_object (pDesklet, NOTIFICATION_UPDATE, (CairoDockNotificationFunc) on_update_desklet, CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_MOUSE_MOVED, (CairoDockNotificationFunc) on_mouse_move, CAIRO_DOCK_RUN_AFTER, NULL);
	
	return pCaroussel;
}

static void load_data (CairoDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	//g_print ("%s (%dx%d)\n", __func__, pDesklet->container.iWidth, pDesklet->container.iHeight);
	
	int iMaxIconWidth = 0;
	Icon *icon;
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		iMaxIconWidth = MAX (iMaxIconWidth, icon->fWidth);
	}
	
	double fCentralSphereWidth, fCentralSphereHeight;
	if (pCaroussel->b3D && ! g_bUseOpenGL)
	{
		fCentralSphereWidth = MAX (1, MIN (pDesklet->container.iWidth/3, pDesklet->container.iHeight/2));
		fCentralSphereHeight = fCentralSphereWidth;
		
		pCaroussel->iEllipseHeight = MIN (fCentralSphereHeight, pDesklet->container.iHeight - 2 * (myIconsParam.iconTextDescription.iSize + myIconsParam.fReflectSize) - 1);
		pCaroussel->iFrameHeight = MIN (pCaroussel->iEllipseHeight + myIconsParam.fReflectSize, pDesklet->container.iHeight);
		pCaroussel->fInclinationOnHorizon = atan2 (pDesklet->container.iWidth/4, pCaroussel->iFrameHeight);
		pCaroussel->fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pCaroussel->iFrameHeight, pCaroussel->fInclinationOnHorizon, myDocksParam.iDockRadius, myDocksParam.iDockLineWidth);
		
		pCaroussel->a = MAX (pDesklet->container.iWidth - pCaroussel->fExtraWidth - (pCaroussel->bRotateIconsOnEllipse ? 0 : iMaxIconWidth/2), pCaroussel->iEllipseHeight)/2;
		pCaroussel->b = MIN (pDesklet->container.iWidth - pCaroussel->fExtraWidth - (pCaroussel->bRotateIconsOnEllipse ? 0 : iMaxIconWidth/2), pCaroussel->iEllipseHeight)/2;  // c = sqrt (a * a - b * b) ; e = c / a.
	}
	else if (!pCaroussel->b3D)
	{
		fCentralSphereWidth = MAX (1, pDesklet->container.iWidth * CAROUSSEL_RATIO_MAIN_ICON_DESKLET);
		fCentralSphereHeight = MAX (1, pDesklet->container.iHeight * CAROUSSEL_RATIO_MAIN_ICON_DESKLET);
		
		pCaroussel->a = MAX (fCentralSphereWidth, fCentralSphereHeight)/2 + .1*pDesklet->container.iWidth;
		pCaroussel->b = MIN (fCentralSphereWidth, fCentralSphereHeight)/2 + .1*pDesklet->container.iHeight;
	}
	else
	{
		fCentralSphereWidth = MAX (1, MIN (pDesklet->container.iWidth/3, pDesklet->container.iHeight/2));
		pCaroussel->a = pDesklet->container.iWidth/4;
		pCaroussel->b = fCentralSphereWidth/2 + .1*pDesklet->container.iWidth;
	}
}


static void free_data (CairoDesklet *pDesklet)
{
	cairo_dock_remove_notification_func_on_object (pDesklet, NOTIFICATION_UPDATE, (CairoDockNotificationFunc) on_update_desklet, NULL);
	cairo_dock_remove_notification_func_on_object (CAIRO_CONTAINER (pDesklet), NOTIFICATION_MOUSE_MOVED, (CairoDockNotificationFunc) on_mouse_move, NULL);
	
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	
	g_free (pCaroussel);
	pDesklet->pRendererData = NULL;
}

static void set_icon_size (CairoDesklet *pDesklet, Icon *pIcon)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	
	double fCentralSphereWidth = MAX (1, MIN (pDesklet->container.iWidth/3, pDesklet->container.iHeight/2));
	if (pIcon == pDesklet->pIcon)
	{
		if (pCaroussel->b3D)
		{
			pIcon->fWidth = fCentralSphereWidth;
			pIcon->fHeight = pIcon->fWidth;
		}
		else
		{
			pIcon->fWidth = MAX (1, pDesklet->container.iWidth * CAROUSSEL_RATIO_MAIN_ICON_DESKLET);
			pIcon->fHeight = MAX (1, pDesklet->container.iHeight * CAROUSSEL_RATIO_MAIN_ICON_DESKLET);
		}
	}
	else
	{
		if (pCaroussel->b3D)
		{
			pIcon->fWidth = fCentralSphereWidth/2;  // lorsque l'icone est devant, la ou elle est la plus grosse.
			pIcon->fHeight = pIcon->fWidth;
		}
		else
		{
			pIcon->fWidth = MAX (1, .2 * pDesklet->container.iWidth - myIconsParam.iconTextDescription.iSize);
			pIcon->fHeight = MAX (1, .2 * pDesklet->container.iHeight - myIconsParam.iconTextDescription.iSize);
		}
	}
}

static void calculate_icons (CairoDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	
	int iNbIcons = g_list_length (pDesklet->icons);
	pCaroussel->fDeltaTheta = (iNbIcons != 0 ? 2 * G_PI / iNbIcons : 0);
	
	// on calcule l'icone du centre.
	Icon *pIcon = pDesklet->pIcon;
	double fCentralSphereWidth = MAX (1, MIN (pDesklet->container.iWidth/3, pDesklet->container.iHeight/2));
	if (pIcon != NULL)
	{
		if (pCaroussel->b3D)
		{
			pIcon->fWidth = fCentralSphereWidth;
			pIcon->fHeight = pIcon->fWidth;
		}
		else
		{
			pIcon->fWidth = MAX (1, pDesklet->container.iWidth * CAROUSSEL_RATIO_MAIN_ICON_DESKLET);
			pIcon->fHeight = MAX (1, pDesklet->container.iHeight * CAROUSSEL_RATIO_MAIN_ICON_DESKLET);
		}
		//pIcon->iImageWidth = pIcon->fWidth;
		//pIcon->iImageHeight = pIcon->fHeight;
		
		pIcon->fDrawX = (pDesklet->container.iWidth - pIcon->fWidth) / 2;
		pIcon->fDrawY = (pDesklet->container.iHeight - pIcon->fHeight) / 2 + (pCaroussel->b3D ? myIconsParam.iconTextDescription.iSize : 0);
		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->fGlideScale = 1.;
	}
	
	// on calcule le cortege d'icones.
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pCaroussel->b3D)
		{
			pIcon->fWidth = fCentralSphereWidth/2;  // lorsque l'icone est devant, la ou elle est la plus grosse.
			pIcon->fHeight = pIcon->fWidth;
		}
		else
		{
			pIcon->fWidth = MAX (1, .2 * pDesklet->container.iWidth - myIconsParam.iconTextDescription.iSize);
			pIcon->fHeight = MAX (1, .2 * pDesklet->container.iHeight - myIconsParam.iconTextDescription.iSize);
		}
		//pIcon->iImageWidth = pIcon->fWidth;
		//pIcon->iImageHeight = pIcon->fHeight;
		
		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->fGlideScale = 1.;
	}
}



static void render (cairo_t *pCairoContext, CairoDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	//g_print ("%s(%x)\n", __func__, pCaroussel);
	if (pCaroussel == NULL)
		return ;
	
	double fTheta = G_PI/2 + pCaroussel->fRotationAngle, fDeltaTheta = pCaroussel->fDeltaTheta;
	
	int iEllipseHeight = pCaroussel->iEllipseHeight;
	double fInclinationOnHorizon = pCaroussel->fInclinationOnHorizon;
	
	int iFrameHeight = pCaroussel->iFrameHeight;
	double fExtraWidth = pCaroussel->fExtraWidth;
	double a = pCaroussel->a, b = pCaroussel->b;
	
	Icon *pIcon;
	GList *ic;
	if (pCaroussel->b3D)
	{
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			
			if (fTheta > G_PI && fTheta < 2*G_PI)  // arriere-plan.
			{
				pIcon->fScale = (1 + .5 * fabs (fTheta - 3 * G_PI / 2) / (G_PI / 2)) / 1.5;
				pIcon->fAlpha = pIcon->fScale;
			}
			else
			{
				pIcon->fScale = 1.;
				pIcon->fAlpha = 1.;
			}
			pIcon->fDrawX = pDesklet->container.iWidth / 2 + a * cos (fTheta) - pIcon->fWidth/2 * 1;
			pIcon->fDrawY = pDesklet->container.iHeight / 2 + b * sin (fTheta) - pIcon->fHeight * pIcon->fScale + myIconsParam.iconTextDescription.iSize;
			
			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}
		
		//\____________________ On trace le cadre.
		double fLineWidth = myDocksParam.iDockLineWidth;
		double fMargin = 0;
		
		double fDockWidth = pDesklet->container.iWidth - fExtraWidth;
		int sens=1;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = fExtraWidth / 2;
		fDockOffsetY = (pDesklet->container.iHeight - iEllipseHeight) / 2 + myIconsParam.iconTextDescription.iSize;
		
		cairo_save (pCairoContext);
		cairo_dock_draw_frame (pCairoContext, myDocksParam.iDockRadius, fLineWidth, fDockWidth, iFrameHeight, fDockOffsetX, fDockOffsetY, sens, fInclinationOnHorizon, pDesklet->container.bIsHorizontal, TRUE);
		
		//\____________________ On dessine les decorations dedans.
		cairo_set_source_rgba (pCairoContext, .8, .8, .8, .75);
		cairo_fill_preserve (pCairoContext);
		
		//\____________________ On dessine le cadre.
		if (fLineWidth > 0)
		{
			cairo_set_line_width (pCairoContext, fLineWidth);
			cairo_set_source_rgba (pCairoContext, .9, .9, .9, 1.);
			cairo_stroke (pCairoContext);
		}
		cairo_restore (pCairoContext);
		
		//\____________________ On dessine les icones dans l'ordre qui va bien.
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight < pDesklet->container.iHeight / 2 + myIconsParam.iconTextDescription.iSize && pIcon->fDrawX + pIcon->fWidth/2 > pDesklet->container.iWidth / 2)  // arriere-plan droite.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->container.iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight < pDesklet->container.iHeight / 2 + myIconsParam.iconTextDescription.iSize && pIcon->fDrawX + pIcon->fWidth/2 <= pDesklet->container.iWidth / 2)  // arriere-plan gauche.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->container.iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
		
		cairo_save (pCairoContext);
		pDesklet->pIcon->fDrawY = pDesklet->container.iHeight/2 - pDesklet->pIcon->fHeight + myIconsParam.iconTextDescription.iSize;
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, TRUE, FALSE, pDesklet->container.iWidth);
		cairo_restore (pCairoContext);
		
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight >= pDesklet->container.iHeight / 2 + myIconsParam.iconTextDescription.iSize && pIcon->fDrawX + pIcon->fWidth/2 > pDesklet->container.iWidth / 2)  // avant-plan droite.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->container.iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
			
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight >= pDesklet->container.iHeight / 2 + myIconsParam.iconTextDescription.iSize && pIcon->fDrawX + pIcon->fWidth/2 <= pDesklet->container.iWidth / 2)  // avant-plan gauche.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->container.iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
	}
	else
	{
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, FALSE, FALSE, pDesklet->container.iWidth);
		cairo_restore (pCairoContext);
		
		gboolean bFlip = (pDesklet->pIcon->fHeight > pDesklet->pIcon->fWidth);
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				pIcon->fDrawX = pDesklet->pIcon->fDrawX + pDesklet->pIcon->fWidth / 2 + (bFlip ? b : a) * cos (fTheta) - pIcon->fWidth/2;
				pIcon->fDrawY = pDesklet->pIcon->fDrawY + pDesklet->pIcon->fHeight / 2 + (bFlip ? a : b) * sin (fTheta) - pIcon->fHeight/2 + myIconsParam.iconTextDescription.iSize;
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, pDesklet->container.iWidth);
				
				cairo_restore (pCairoContext);
			}
			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}
	}
}

static gint _caroussel_compare_icons_depths(gconstpointer a, gconstpointer b)
{
	const _CarousselPositionedIcon *pSortedIcon1 = (const _CarousselPositionedIcon*)a;
	const _CarousselPositionedIcon *pSortedIcon2 = (const _CarousselPositionedIcon*)b;

	// calcul de la profondeur pour ces icones
	double Zicon1 = sin (pSortedIcon1->fTheta);
	double Zicon2 = sin (pSortedIcon2->fTheta);

	if( Zicon1 < Zicon2 ) return -1;
	else if( Zicon1 > Zicon2 ) return 1;

	return 0;
}

static void _draw_disc_caroussel(CairoDesklet *pDesklet, double fTheta, double a, double b, gboolean bOnlyStencil)
{
	if( bOnlyStencil )
	{
		// On active le stencil pour dessiner aussi dans ce buffer
		glDisable(GL_DEPTH_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		/* Draw 1 into the stencil buffer. */
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
	}

	//\________ Dessiner un disque en dessous du caroussel			
	glBegin(GL_TRIANGLE_FAN);
		glColor4f(0., 0., 0., 0.);
		glVertex3f (0, 0., 0);
		for( int iIter = 0; iIter <= 30; iIter++  )
		{
			glColor4f (0.1, 0.1, ((iIter&1) ? 0.5 : 0.2), 0.5);
			glVertex3f (1.25*a*sin(fTheta+2*G_PI*(double)iIter/30.), 0., 1.25*b*cos(fTheta+2*G_PI*(double)iIter/30.));
		}
	glEnd();
	glColor4f(1., 1., 1., 1.);

	if( bOnlyStencil )
	{
		/* Re-enable update of color and depth. */ 
		glDisable(GL_STENCIL_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}
}

static void render_opengl (CairoDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	
	double fTheta = G_PI/2 + pCaroussel->fRotationAngle, fDeltaTheta = pCaroussel->fDeltaTheta;
	double a = pCaroussel->a, b = pCaroussel->b;

	Icon *pIcon;
	GList *ic, *ic2;

	if (pCaroussel->b3D)
	{
		//a = fCentralSphereWidth/2 + .1*pDesklet->container.iWidth;
		//b = fCentralSphereHeight/2 + .1*pDesklet->container.iHeight;
		glPushMatrix ();
		glEnable(GL_DEPTH_TEST);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // transparence.

		//\____________________ On dessine l'icone au milieu, mais seulement les parties opaques
		glTranslatef( 0., 0.5*b, 0. ); // on se decale un peu plus vers le haut

		  glAlphaFunc ( GL_GREATER, 0.1 ) ;
		glEnable ( GL_ALPHA_TEST ) ;
			_render_one_icon_and_quickinfo_opengl (pDesklet->pIcon, CAIRO_CONTAINER (pDesklet), FALSE);
		glDisable ( GL_ALPHA_TEST ) ;

		glTranslatef( 0., -0.5*b, 0. );
		//glRotatef( 10., 1., 0., 0. );

		// On se met a la bonne hauteur pour le plan, c-a-d en dessous des quickinfos
		if( pDesklet->icons )
		{
			pIcon = (Icon *)(pDesklet->icons->data);
			glTranslatef( 0., -pIcon->fHeight/2, 0. );
		}
		else
		{
			// Au cas ou il n'y a aucune sous-icone !
			glTranslatef( 0., -pDesklet->pIcon->fHeight/2., 0. );
		}

		glPolygonMode (GL_FRONT, GL_FILL);
		
		//\________ Dessiner un disque en dessous du caroussel dans le stencil
		_draw_disc_caroussel(pDesklet, fTheta, a, b, TRUE);

		//\________ On trie les icones par profondeur
		GList *pListSortedIcons = NULL;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			_CarousselPositionedIcon *pSortedIcon = g_new0 (_CarousselPositionedIcon, 1);
			pSortedIcon->pIcon = (Icon *)(ic->data);
			pSortedIcon->fTheta = fTheta;

			pListSortedIcons = g_list_insert_sorted(pListSortedIcons, pSortedIcon, _caroussel_compare_icons_depths);
			
			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}

		//\____________________ On dessine les icones autour: d'abord les reflets....
		
		// Ne pas deborder du disque --> on utilise le buffer du stencil
		glEnable (GL_STENCIL_TEST);
		glStencilFunc (GL_EQUAL, 1, 1);
		glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);

		for (ic = pListSortedIcons; ic != NULL; ic = ic->next)
		{
			_CarousselPositionedIcon *pSortedIcon = ic->data;
			pIcon = pSortedIcon->pIcon;
			fTheta = pSortedIcon->fTheta;
			double previousAlphaIcon = pIcon->fAlpha;

			glPushMatrix ();
			
			//\____________________ On se decale au bon endroit
			glTranslatef (-a * cos (fTheta),
										-pIcon->fHeight/2./*-pIcon->iQuickInfoHeight*/,
										b/2 * sin (fTheta));

			//\____________________ Un reflet, c'est inverse --> on inverse
			glScalef( 1, -1, 1 );

			//\____________________ On calcule la transparence qui va bien
			//  ici on se base sur la profondeur, representee par sin(fTheta) ici
			//    Si sin(fTheta)+0.4 > 1., donc si l'objet est assez proche de nous ==> opaque
			//    Si sin(fTheta)+0.4 < 0., donc assez profond ==> on cache
			double alphaIcon = MAX(MIN(sin (fTheta) + 0.4, 1.), 0.2);

			//\____________________ On met le reflet un peu transparent
			pIcon->fAlpha = alphaIcon * 0.4;

			//\____________________ Et on dessine l'icone
			_render_one_icon_and_quickinfo_opengl (pIcon, CAIRO_CONTAINER (pDesklet), TRUE);
			pIcon->fAlpha = previousAlphaIcon;
			
			glPopMatrix ();
		}
		glDisable (GL_STENCIL_TEST);
		glClear( GL_STENCIL_BUFFER_BIT );

		//\________ Dessiner un disque en dessous du caroussel au dessus des reflets
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // rend le cube transparent.
		_draw_disc_caroussel(pDesklet, fTheta, a, b, FALSE);

		//\____________________ On dessine les icones autour: les "vraies" icones !
		for (ic = pListSortedIcons; ic != NULL; ic = ic->next)
		{
			_CarousselPositionedIcon *pSortedIcon = ic->data;
			pIcon = pSortedIcon->pIcon;
			fTheta = pSortedIcon->fTheta;
			double previousAlphaIcon = pIcon->fAlpha;
			
			glPushMatrix ();

			//\____________________ On se decale au bon endroit
			glTranslatef (-a * cos (fTheta),
				pIcon->fHeight/2/* + pIcon->iQuickInfoHeight*/,
				b/2 * sin (fTheta));

			//\____________________ On se remet droit
			//glRotatef( -10., 1., 0., 0. );

			//\____________________ On calcule la transparence qui va bien
			//  ici on se base sur la profondeur, representee par sin(fTheta) ici
			//    Si sin(fTheta)+0.4 > 1., donc si l'objet est assez proche de nous ==> opaque
			//    Si sin(fTheta)+0.4 < 0., donc assez profond ==> on cache
			double alphaIcon = MAX(MIN(sin (fTheta) + 0.4, 1.), 0.2);

			pIcon->fAlpha *= alphaIcon;
			
			//\____________________ Et on dessine l'icone
			_render_one_icon_and_quickinfo_opengl (pIcon, CAIRO_CONTAINER (pDesklet), FALSE);
			pIcon->fAlpha = previousAlphaIcon;
			
			glPopMatrix ();
		}
		
		glDisable(GL_DEPTH_TEST);
		glDisable (GL_BLEND);
		glPopMatrix ();
		g_list_free (pListSortedIcons);
	}
	else
	{
		//\____________________ On dessine l'icone au milieu
		glPushMatrix ();
		//glScalef( 0.8, 0.8, 0 );
		_render_one_icon_and_quickinfo_opengl (pDesklet->pIcon, CAIRO_CONTAINER (pDesklet), FALSE);
		glPopMatrix ();

		//\____________________ On dessine les icones autour
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			
			glPushMatrix ();
			
			//\____________________ On se decale au bon endroit
			glTranslatef (a * cos (fTheta),
										b * sin (fTheta),
										0.);

			//glScalef( 0.8, 0.8, 0 );

			//\____________________ Et on dessine l'icone
			_render_one_icon_and_quickinfo_opengl (pIcon, CAIRO_CONTAINER (pDesklet), FALSE);
			
			glPopMatrix ();

			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}
	}
}


static void render_bounding_box (CairoDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	
	double fTheta = G_PI/2 + pCaroussel->fRotationAngle, fDeltaTheta = pCaroussel->fDeltaTheta;
	double a = pCaroussel->a, b = pCaroussel->b;
	
	double x, y, w, h;
	Icon *pIcon;
	if (pCaroussel->b3D)
	{
		glEnable(GL_DEPTH_TEST);
		glTranslatef( 0., 0.5*b, 0. ); // on se decale un peu plus vers le haut
		pIcon = pDesklet->pIcon;
		if (pIcon != NULL && pIcon->iIconTexture != 0)  // l'icone au centre.
		{
			w = pIcon->fWidth/2;
			h = pIcon->fHeight/2;
			
			glLoadName(pIcon->iIconTexture);
			
			glBegin(GL_QUADS);
			glVertex3f(-w, +h, 0.);
			glVertex3f(+w, +h, 0.);
			glVertex3f(+w, -h, 0.);
			glVertex3f(-w, -h, 0.);
			glEnd();
		}
		
		glTranslatef( 0., -0.5*b, 0. );
		if( pDesklet->icons )
		{
			pIcon = (Icon *)(pDesklet->icons->data);
			glTranslatef( 0., -pIcon->fHeight/2, 0. );
		}
		else
		{
			// Au cas ou il n'y a aucune sous-icone !
			glTranslatef( 0., -pDesklet->pIcon->fHeight/2., 0. );
		}
		GList *ic;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)  // les icones autour du centre.
		{
			pIcon = ic->data;
			if (pIcon->iIconTexture == 0)
				continue;
			
			glPushMatrix ();
			glTranslatef (-a * cos (fTheta),
				pIcon->fHeight/2/* + pIcon->iQuickInfoHeight*/,
				b/2 * sin (fTheta));
			
			w = pIcon->fWidth/2;
			h = pIcon->fHeight/2;
			glLoadName(pIcon->iIconTexture);
			
			glBegin(GL_QUADS);
			glVertex3f(-w, +h, 0.);
			glVertex3f(+w, +h, 0.);
			glVertex3f(+w, -h, 0.);
			glVertex3f(-w, -h, 0.);
			glEnd();
			glPopMatrix ();
			
			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}
		glDisable(GL_DEPTH_TEST);
	}
	else
	{
		pIcon = pDesklet->pIcon;
		if (pIcon != NULL && pIcon->iIconTexture != 0)  // l'icone au centre.
		{
			w = pIcon->fWidth/2;
			h = pIcon->fHeight/2;
			x = 0.;
			y = 0.;
			
			glLoadName(pIcon->iIconTexture);
			
			glBegin(GL_QUADS);
			glVertex3f(x-w, y+h, 0.);
			glVertex3f(x+w, y+h, 0.);
			glVertex3f(x+w, y-h, 0.);
			glVertex3f(x-w, y-h, 0.);
			glEnd();
		}
		
		GList *ic;
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)  // les icones autour du centre.
		{
			pIcon = ic->data;
			if (pIcon->iIconTexture == 0)
				continue;
			
			w = pIcon->fWidth/2;
			h = pIcon->fHeight/2;
			x = a * cos (fTheta);
			y = b * sin (fTheta);
			
			glLoadName(pIcon->iIconTexture);
			
			glBegin(GL_QUADS);
			glVertex3f(x-w, y+h, 0.);
			glVertex3f(x+w, y+h, 0.);
			glVertex3f(x+w, y-h, 0.);
			glVertex3f(x-w, y-h, 0.);
			glEnd();

			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}
	}
}

void rendering_register_caroussel_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render 			= (CairoDeskletRenderFunc) render;
	pRenderer->configure 		= (CairoDeskletConfigureRendererFunc) configure;
	pRenderer->load_data 		= (CairoDeskletLoadRendererDataFunc) load_data;
	pRenderer->free_data 		= (CairoDeskletFreeRendererDataFunc) free_data;
	pRenderer->calculate_icons	= (CairoDeskletCalculateIconsFunc) calculate_icons;
	pRenderer->render_opengl 	= (CairoDeskletGLRenderFunc) render_opengl;
	pRenderer->render_bounding_box 	= (CairoDeskletGLRenderFunc) render_bounding_box;
	
	cairo_dock_register_desklet_renderer ("Caroussel", pRenderer);
}
