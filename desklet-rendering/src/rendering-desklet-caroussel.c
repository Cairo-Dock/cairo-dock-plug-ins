/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <math.h>
#include <cairo-dock.h>

#include "rendering-desklet-caroussel.h"

#define CAROUSSEL_RATIO_ICON_DESKLET .5

// Pour dessiner une icone, avec son quickinfo, dans la matrice courante.
void _render_one_icon_and_quickinfo_opengl (Icon *pIcon, CairoContainer *pContainer, gboolean bIsReflect)
{
	if (pIcon == NULL)  // peut arriver avant de lier l'icone au desklet.
		return ;

	if (pIcon->iIconTexture != 0)
	{
		glPushMatrix ();
			cairo_dock_draw_icon_texture (pIcon, pContainer);
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

static void _caroussel_rotate_delta(CairoDesklet *pDesklet, double fDeltaTheta)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return;
	pCaroussel->fRotationAngle += pCaroussel->iRotationDirection * fDeltaTheta;
	if (pCaroussel->fRotationAngle >= 2*G_PI)
		pCaroussel->fRotationAngle -= 2*G_PI;
	else if (pCaroussel->fRotationAngle < 0)
		pCaroussel->fRotationAngle += 2*G_PI;
	gtk_widget_queue_draw (pDesklet->pWidget);
}

static gboolean _caroussel_rotate (CairoDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return FALSE;

	_caroussel_rotate_delta(pDesklet, pCaroussel->fDeltaTheta / 10.);

	pCaroussel->iRotationCount += (pCaroussel->iRotationDirection == GDK_SCROLL_UP ? 1 : -1);
	if (abs (pCaroussel->iRotationCount) >= 10 || pCaroussel->iRotationCount == 0)
	{
		pCaroussel->iRotationCount = 0;
		pCaroussel->iSidRotation = 0;
		return FALSE;
	}
	else
		return TRUE;
}

gboolean on_scroll_desklet(gpointer pUserData, Icon *pClickedIcon, CairoDesklet *pDesklet, int iDirection)
{
	if (! pDesklet->pRenderer || pDesklet->pRenderer->render != rendering_draw_caroussel_in_desklet)
	//if( pDesklet != pUserData || (pDesklet && !pDesklet->bInside))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pDesklet->icons != NULL && (iDirection == GDK_SCROLL_DOWN || iDirection == GDK_SCROLL_UP))
	{
		CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
		if (pCaroussel == NULL)
			return FALSE;
		
		if (pCaroussel->iSidRotation == 0)
		{
			pCaroussel->iRotationDirection = iDirection;
			pCaroussel->iSidRotation = g_timeout_add (50, (GSourceFunc) _caroussel_rotate, (gpointer) pDesklet);
		}
		else
		{
			if (iDirection == pCaroussel->iRotationDirection)
			{
				pCaroussel->iRotationCount = 0;
			}
			else
			{
				pCaroussel->iRotationDirection = iDirection;
			}
		}
		_caroussel_rotate (pDesklet);
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

//gboolean cd_drop_indicator_mouse_moved (gpointer pUserData, CairoDock *pDock, gboolean *bStartAnimation);
//gboolean cd_drop_indicator_update_dock (gpointer pUserData, CairoDesklet *pDock, gboolean *bContinueAnimation);

gboolean on_motion_desklet (gpointer pUserData, CairoDesklet *pDesklet, gboolean *bStartAnimation)
{
	cd_message( "pUserData = %x, pDesklet = %x", pUserData, pDesklet );

	if (! pDesklet->pRenderer || pDesklet->pRenderer->render != rendering_draw_caroussel_in_desklet || ! pDesklet->bInside)
	//if( pDesklet != pUserData || (pDesklet && !pDesklet->bInside) )
	  return CAIRO_DOCK_LET_PASS_NOTIFICATION;

	 cd_message( "this is our stuff !");
	
	if (pDesklet->icons != NULL)
	{
		CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
		if (pCaroussel == NULL)
			return FALSE;

		// si on est dans les entre 0% et 30% de la largeur du desklet,
		// alors on tourne a droite (1)
		if( pDesklet->iMouseX <= pDesklet->iWidth*0.3 )
		{
			pCaroussel->iRotationDirection = 1;
			// La force de rotation va de 0 (lorsqu'on est a 30%) jusqu'a
			// pCaroussel->fDeltaTheta / 10. (lorsqu'on est a 0%)
			double fDeltaRotation = (pCaroussel->fDeltaTheta / 10) *
			                        (pDesklet->iWidth*0.3 - pDesklet->iMouseX)/(pDesklet->iWidth*0.3);
			_caroussel_rotate_delta( pDesklet, fDeltaRotation );
			*bStartAnimation = TRUE;
		}
		// si on est dans les entre 80% et 100% de la largeur du desklet,
		// alors on tourne a gauche (-1)
		else if( pDesklet->iMouseX >= pDesklet->iWidth*0.7 )
		{
			// La force de rotation va de 0 (lorsqu'on est a 70%) jusqu'a
			// pCaroussel->fDeltaTheta / 10. (lorsqu'on est a 100%)
			double fDeltaRotation = (pCaroussel->fDeltaTheta / 10) *
			                        (pDesklet->iMouseX - pDesklet->iWidth*0.7)/(pDesklet->iWidth*0.3);
			pCaroussel->iRotationDirection = -1;
			_caroussel_rotate_delta( pDesklet, fDeltaRotation );
			*bStartAnimation = TRUE;
		}
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

/*gboolean on_enter_desklet (gpointer pUserData, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	CairoDesklet *pDesklet = CAIRO_DESKLET (pContainer);
	if( pDesklet != pUserData || (pDesklet && !pDesklet->bInside) )
	  return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	*bStartAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}*/


CDCarousselParameters *rendering_configure_caroussel (CairoDesklet *pDesklet, cairo_t *pSourceContext, gpointer *pConfig)
{
	GList *pIconsList = pDesklet->icons;
	
	CDCarousselParameters *pCaroussel = g_new0 (CDCarousselParameters, 1);
	if (pConfig != NULL)
	{
		pCaroussel->b3D = GPOINTER_TO_INT (pConfig[0]);
		pCaroussel->bRotateIconsOnEllipse = GPOINTER_TO_INT (pConfig[1]);
	}
	
	int iNbIcons = g_list_length (pDesklet->icons);
	pCaroussel->fDeltaTheta = (iNbIcons != 0 ? 2 * G_PI / iNbIcons : 0);
	
	return pCaroussel;
}

void rendering_load_caroussel_data (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	
	int iMaxIconWidth = 0;
	Icon *icon;
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		iMaxIconWidth = MAX (iMaxIconWidth, icon->fWidth);
	}
	
	double fCentralSphereWidth, fCentralSphereHeight;
	if (pCaroussel->b3D)
	{
		fCentralSphereWidth = MAX (1, MIN (pDesklet->iWidth, pDesklet->iHeight) * CAROUSSEL_RATIO_ICON_DESKLET);
		fCentralSphereHeight = fCentralSphereWidth;
		
		pCaroussel->iEllipseHeight = MIN (fCentralSphereHeight, pDesklet->iHeight - 2 * (myLabels.iconTextDescription.iSize + myIcons.fReflectSize) - 1);
		pCaroussel->fInclinationOnHorizon = atan2 (pDesklet->iHeight, pDesklet->iWidth/4);
		pCaroussel->iFrameHeight = pCaroussel->iEllipseHeight + 0*2 * myBackground.iFrameMargin + myIcons.fReflectSize;
		pCaroussel->fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (pCaroussel->iFrameHeight, pCaroussel->fInclinationOnHorizon, g_iDockRadius, g_iDockLineWidth);
		
		pCaroussel->a = MAX (pDesklet->iWidth - pCaroussel->fExtraWidth - (pCaroussel->bRotateIconsOnEllipse ? 0 : iMaxIconWidth/2), pCaroussel->iEllipseHeight)/2;
		pCaroussel->b = MIN (pDesklet->iWidth - pCaroussel->fExtraWidth - (pCaroussel->bRotateIconsOnEllipse ? 0 : iMaxIconWidth/2), pCaroussel->iEllipseHeight)/2;  // c = sqrt (a * a - b * b) ; e = c / a.
	}
	else
	{
		fCentralSphereWidth = MAX (1, (pDesklet->iWidth - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
		fCentralSphereHeight = MAX (1, (pDesklet->iHeight - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
		
		pCaroussel->a = MAX (fCentralSphereWidth, fCentralSphereHeight)/2 + .1*pDesklet->iWidth;
		pCaroussel->b = MIN (fCentralSphereWidth, fCentralSphereHeight)/2 + .1*pDesklet->iHeight;
	}
	
	/*// brancher la rotation sur la position de la souris, lors de l'update du desklet
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_DESKLET, (CairoDockNotificationFunc) on_motion_desklet, CAIRO_DOCK_RUN_AFTER, pDesklet);
	//  on garde quand meme la notif pour le scroll, pour les nostalgiques
	cairo_dock_register_notification (CAIRO_DOCK_SCROLL_ICON, (CairoDockNotificationFunc) on_scroll_desklet, CAIRO_DOCK_RUN_AFTER, pDesklet);
	//cairo_dock_register_notification (CAIRO_DOCK_ENTER_DESKLET, (CairoDockNotificationFunc) on_enter_desklet, CAIRO_DOCK_RUN_AFTER, NULL);*/
}


void rendering_free_caroussel_data (CairoDesklet *pDesklet)
{
//	cairo_dock_remove_notification_func (CAIRO_DOCK_MOUSE_MOVED, (CairoDockNotificationFunc) on_motion_desklet, pDesklet);
	/*cairo_dock_remove_notification_func (CAIRO_DOCK_UPDATE_DESKLET, (CairoDockNotificationFunc) on_motion_desklet, pDesklet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_SCROLL_ICON, (CairoDockNotificationFunc) on_scroll_desklet, pDesklet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_ENTER_DESKLET, (CairoDockNotificationFunc) on_enter_desklet, NULL);*/
	
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	
	g_free (pCaroussel);
	pDesklet->pRendererData = NULL;
}


void rendering_load_icons_for_caroussel (CairoDesklet *pDesklet, cairo_t *pSourceContext)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	if (pCaroussel == NULL)
		return ;
	
	Icon *pIcon = pDesklet->pIcon;
	if (pIcon != NULL)
	{
		if (pCaroussel->b3D)
		{
			pIcon->fWidth = MAX (1, MIN (pDesklet->iWidth, pDesklet->iHeight) * CAROUSSEL_RATIO_ICON_DESKLET);
			pIcon->fHeight = pIcon->fWidth;
		}
		else
		{
			pIcon->fWidth = MAX (1, (pDesklet->iWidth - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
			pIcon->fHeight = MAX (1, (pDesklet->iHeight - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
		}
		
		pIcon->fDrawX = (pDesklet->iWidth - pIcon->fWidth) / 2;
		pIcon->fDrawY = (pDesklet->iHeight - pIcon->fHeight) / 2 + (pCaroussel->b3D ? myLabels.iconTextDescription.iSize : 0);
		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->fGlideScale = 1.;
		cairo_dock_fill_icon_buffers_for_desklet (pIcon, pSourceContext);
	}
	GList* ic;
	for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pCaroussel->b3D)
		{
			pIcon->fWidth = MAX (1, MIN (pDesklet->iWidth, pDesklet->iHeight) * .25);
			pIcon->fHeight = pIcon->fWidth;
		}
		else
		{
			pIcon->fWidth = MAX (1, .2 * pDesklet->iWidth - myLabels.iconTextDescription.iSize);
			pIcon->fHeight = MAX (1, .2 * pDesklet->iHeight - myLabels.iconTextDescription.iSize);
		}

		pIcon->fScale = 1.;
		pIcon->fAlpha = 1.;
		pIcon->fWidthFactor = 1.;
		pIcon->fHeightFactor = 1.;
		pIcon->fGlideScale = 1.;
		
		cairo_dock_fill_icon_buffers_for_desklet (pIcon, pSourceContext);
	}
}



void rendering_draw_caroussel_in_desklet (cairo_t *pCairoContext, CairoDesklet *pDesklet, gboolean bRenderOptimized)
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
			pIcon->fDrawX = pDesklet->iWidth / 2 + a * cos (fTheta) - pIcon->fWidth/2 * 1;
			pIcon->fDrawY = pDesklet->iHeight / 2 + b * sin (fTheta) - pIcon->fHeight * pIcon->fScale + myLabels.iconTextDescription.iSize;
			
			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}
		
		//\____________________ On trace le cadre.
		double fLineWidth = g_iDockLineWidth;
		double fMargin = 0*myBackground.iFrameMargin;
		
		double fDockWidth = pDesklet->iWidth - fExtraWidth;
		int sens=1;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = fExtraWidth / 2;
		fDockOffsetY = (pDesklet->iHeight - iEllipseHeight) / 2 + myLabels.iconTextDescription.iSize;
		
		cairo_save (pCairoContext);
		cairo_dock_draw_frame (pCairoContext, g_iDockRadius, fLineWidth, fDockWidth, iFrameHeight, fDockOffsetX, fDockOffsetY, sens, fInclinationOnHorizon, pDesklet->bIsHorizontal);
		
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
				
				if (pIcon->fDrawY + pIcon->fHeight < pDesklet->iHeight / 2 + myLabels.iconTextDescription.iSize && pIcon->fDrawX + pIcon->fWidth/2 > pDesklet->iWidth / 2)  // arriere-plan droite.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight < pDesklet->iHeight / 2 + myLabels.iconTextDescription.iSize && pIcon->fDrawX + pIcon->fWidth/2 <= pDesklet->iWidth / 2)  // arriere-plan gauche.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
		
		cairo_save (pCairoContext);
		pDesklet->pIcon->fDrawY = pDesklet->iHeight/2 - pDesklet->pIcon->fHeight + myLabels.iconTextDescription.iSize;
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, TRUE, FALSE, pDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight >= pDesklet->iHeight / 2 + myLabels.iconTextDescription.iSize && pIcon->fDrawX + pIcon->fWidth/2 > pDesklet->iWidth / 2)  // avant-plan droite.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
			
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				if (pIcon->fDrawY + pIcon->fHeight >= pDesklet->iHeight / 2 + myLabels.iconTextDescription.iSize && pIcon->fDrawX + pIcon->fWidth/2 <= pDesklet->iWidth / 2)  // avant-plan gauche.
					cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, pDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
		}
	}
	else
	{
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon_in_desklet (pDesklet->pIcon, pCairoContext, FALSE, FALSE, pDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		gboolean bFlip = (pDesklet->pIcon->fHeight > pDesklet->pIcon->fWidth);
		for (ic = pDesklet->icons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				pIcon->fDrawX = pDesklet->pIcon->fDrawX + pDesklet->pIcon->fWidth / 2 + (bFlip ? b : a) * cos (fTheta) - pIcon->fWidth/2;
				pIcon->fDrawY = pDesklet->pIcon->fDrawY + pDesklet->pIcon->fHeight / 2 + (bFlip ? a : b) * sin (fTheta) - pIcon->fHeight/2 + myLabels.iconTextDescription.iSize;
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, pDesklet->iWidth);
				
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
		glVertex3f (0, -pDesklet->pIcon->fHeight/2., 0);
		for( int iIter = 0; iIter <= 30; iIter++  )
		{
			glColor4f(0.1, 0.1, ((iIter%2)==0)?0.5:0.3, 0.8);
			glVertex3f (1.5*a*sin(fTheta+2*G_PI*(double)iIter/30.), -pDesklet->pIcon->fHeight/2., 1.5*b*cos(fTheta+2*G_PI*(double)iIter/30.));
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

void rendering_draw_caroussel_in_desklet_opengl (CairoDesklet *pDesklet)
{
	CDCarousselParameters *pCaroussel = (CDCarousselParameters *) pDesklet->pRendererData;
	//g_print ("%s(%x)\n", __func__, pCaroussel);
	if (pCaroussel == NULL)
		return ;
	
	double fTheta = G_PI/2 + pCaroussel->fRotationAngle, fDeltaTheta = pCaroussel->fDeltaTheta;
	double a = pCaroussel->a, b = pCaroussel->b;

	Icon *pIcon;
	GList *ic, *ic2;

	if (pCaroussel->b3D)
	{
		double fCentralSphereWidth, fCentralSphereHeight;
		fCentralSphereWidth = MAX (1, (pDesklet->iWidth - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
		fCentralSphereHeight = MAX (1, (pDesklet->iHeight - g_iDockRadius) * CAROUSSEL_RATIO_ICON_DESKLET);
		
		a = fCentralSphereWidth/2 + .1*pDesklet->iWidth;
		b = fCentralSphereHeight/2 + .1*pDesklet->iHeight;

		glPushMatrix ();
		glEnable(GL_DEPTH_TEST);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // rend le cube transparent.

		//\____________________ On dessine l'icone au milieu, mais seulement les parties opaques
		glTranslatef( 0., 0.3*b, 0. ); // on se decale un peu plus vers le haut

	  glAlphaFunc ( GL_GREATER, 0.1 ) ;
    glEnable ( GL_ALPHA_TEST ) ;
		_render_one_icon_and_quickinfo_opengl (pDesklet->pIcon, CAIRO_CONTAINER (pDesklet), FALSE);
    glDisable ( GL_ALPHA_TEST ) ;

		glTranslatef( 0., -0.3*b, -b/2. );
		glRotatef( 10., 1., 0., 0. );

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

			//\__________ On dessine un reflet sur le disque
			glPushMatrix ();
			
			//\____________________ On se decale au bon endroit
			glTranslatef (-a * cos (fTheta),
										pDesklet->pIcon->fHeight/2.+0.1,
										b * sin (fTheta));

			//\____________________ Un reflet, c'est inverse --> on inverse
			glScalef( 1, -1, 1 );

			//\____________________ On met le reflet un peu transparent
			pIcon->fAlpha *= 0.4;			
			//\____________________ Et on dessine l'icone
			_render_one_icon_and_quickinfo_opengl (pIcon, CAIRO_CONTAINER (pDesklet), TRUE);
			pIcon->fAlpha = previousAlphaIcon;
			
			glPopMatrix ();
		}
		glDisable (GL_STENCIL_TEST);
		glClear( GL_STENCIL_BUFFER_BIT );

		//\________ Dessiner un disque en dessous du caroussel au dessus des reflets
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
										0.,
										1.5 * b * sin (fTheta));

			//\____________________ On se remet droit
			glRotatef( -10., 1., 0., 0. );

			//\____________________ On calcule la transparence qui va bien
			//  ici on se base sur la profondeur, representee par sin(fTheta) ici
			//    Si sin(fTheta)+0.4 > 1., donc si l'objet est assez proche de nous ==> opaque
			//    Si sin(fTheta)+0.4 < 0., donc assez profond ==> on cache
			double alphaIcon = MAX(MIN(sin (fTheta) + 0.4, 1.), 0.);

			pIcon->fAlpha *= alphaIcon;
			
			//\____________________ Et on dessine l'icone
			_render_one_icon_and_quickinfo_opengl (pIcon, CAIRO_CONTAINER (pDesklet), FALSE);
			pIcon->fAlpha = previousAlphaIcon;
			
			glPopMatrix ();
		}
		
		glDisable(GL_DEPTH_TEST);
		glDisable (GL_BLEND);
		glPopMatrix ();
	}
	else
	{
		//\____________________ On dessine l'icone au milieu
		glPushMatrix ();
		glScalef( 0.8, 0.8, 0 );
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

			glScalef( 0.8, 0.8, 0 );

			//\____________________ Et on dessine l'icone
			_render_one_icon_and_quickinfo_opengl (pIcon, CAIRO_CONTAINER (pDesklet), FALSE);
			
			glPopMatrix ();

			fTheta += fDeltaTheta;
			if (fTheta >= G_PI/2 + 2*G_PI)
				fTheta -= 2*G_PI;
		}
	}
}



void rendering_register_caroussel_desklet_renderer (void)
{
	CairoDeskletRenderer *pRenderer = g_new0 (CairoDeskletRenderer, 1);
	pRenderer->render = rendering_draw_caroussel_in_desklet;
	pRenderer->configure = rendering_configure_caroussel;
	pRenderer->load_data = rendering_load_caroussel_data;
	pRenderer->free_data = rendering_free_caroussel_data;
	pRenderer->load_icons = rendering_load_icons_for_caroussel;
	pRenderer->render_opengl = rendering_draw_caroussel_in_desklet_opengl;
	
	cairo_dock_register_desklet_renderer (MY_APPLET_CAROUSSEL_DESKLET_RENDERER_NAME, pRenderer);
	
	/// je deplace ca la pour le moment, il faudrait les enregistrer sur le desklet en particulier plutot que tous.
	/// le scroll fait planter ...
	
	// brancher la rotation sur la position de la souris, lors de l'update du desklet
	cairo_dock_register_notification (CAIRO_DOCK_UPDATE_DESKLET, (CairoDockNotificationFunc) on_motion_desklet, CAIRO_DOCK_RUN_AFTER, NULL);
	//  on garde quand meme la notif pour le scroll, pour les nostalgiques
	///cairo_dock_register_notification (CAIRO_DOCK_SCROLL_ICON, (CairoDockNotificationFunc) on_scroll_desklet, CAIRO_DOCK_RUN_AFTER, NULL);
	//cairo_dock_register_notification (CAIRO_DOCK_ENTER_DESKLET, (CairoDockNotificationFunc) on_enter_desklet, CAIRO_DOCK_RUN_AFTER, NULL);
}
