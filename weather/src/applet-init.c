/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"
#include "math.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-init.h"

AppletConfig myConfig;
AppletData myData;
gboolean my_bRotateIconsOnEllipse = TRUE;

CD_APPLET_DEFINITION ("weather", 1, 5, 1)


void cd_weather_draw_in_desklet (cairo_t *pCairoContext, gpointer data)
{
	if (myConfig.bDesklet3D)
	{
		int iNumIcon=0, iNbIcons = g_list_length (myData.pDeskletIconList);
		cd_debug ("%d icones a dessiner", iNbIcons);
		double fTheta = G_PI/2, fDeltaTheta = 2 * G_PI / iNbIcons;
		
		int iEllipseHeight = MIN (myIcon->fHeight, myDesklet->iHeight - 2 * (g_iLabelSize + g_fReflectSize) - 1);
		double fInclinationOnHorizon = 40./180.*G_PI;
		
		int iFrameHeight = iEllipseHeight + 0*2 * g_iFrameMargin + g_fReflectSize;
		double fExtraWidth = cairo_dock_calculate_extra_width_for_trapeze (iFrameHeight, fInclinationOnHorizon, g_iDockRadius, g_iDockLineWidth);
		double a = MAX (myDesklet->iWidth - fExtraWidth - (my_bRotateIconsOnEllipse ? 0 : myData.iMaxIconWidth/2), iEllipseHeight)/2, b = MIN (myDesklet->iWidth - fExtraWidth - (my_bRotateIconsOnEllipse ? 0 : myData.iMaxIconWidth/2), iEllipseHeight)/2;
		double c = sqrt (a * a - b * b);
		double e = c / a;
		gboolean bFlip = (iEllipseHeight > myDesklet->iWidth - fExtraWidth - (my_bRotateIconsOnEllipse ? 0 : myData.iMaxIconWidth/2));
		double fRadius;
		Icon *pIcon;
		GList *ic;
		GList *pFirstDrawnElement = (myData.pFirstDrawnElement != NULL ? myData.pFirstDrawnElement : myData.pDeskletIconList);
		if (pFirstDrawnElement != NULL)
		{
			ic = pFirstDrawnElement;
			do
			{
				pIcon = ic->data;
				fRadius = (bFlip ? sqrt (b * b / (1 - e * e * cos (G_PI/2-fTheta) * cos (G_PI/2-fTheta))) : sqrt (b * b / (1 - e * e * cos (fTheta) * cos (fTheta))));
				pIcon->fDrawX = myDesklet->iWidth / 2 + fRadius * cos (fTheta) - pIcon->fWidth/2;
				pIcon->fDrawY = myDesklet->iHeight / 2 + fRadius * sin (fTheta) - pIcon->fHeight;
				if (fTheta > G_PI && fTheta < 3*G_PI/2)  // arriere-plan.
				{
					pIcon->fScale = MAX (0.75, sin ((G_PI - fabs (fTheta)) / 3));
					pIcon->fAlpha = MAX (0.5, sin (fTheta) * sin (fTheta));
				}
				else
				{
					pIcon->fScale = 1.;
					pIcon->fAlpha = 1.;
				}
				fTheta += fDeltaTheta;
				ic = cairo_dock_get_next_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
		}
		
		//\____________________ On trace le cadre.
		cairo_translate (pCairoContext, 0, g_iLabelSize);
		double fLineWidth = g_iDockLineWidth;
		double fMargin = 0*g_iFrameMargin;
		
		double fDockWidth = myDesklet->iWidth - fExtraWidth;
		int sens=1;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du cadre.
		fDockOffsetX = fExtraWidth / 2;
		fDockOffsetY = (myDesklet->iHeight - iEllipseHeight) / 2;
		
		cairo_save (pCairoContext);
		cairo_dock_draw_frame (pCairoContext, g_iDockRadius, fLineWidth, fDockWidth, iFrameHeight, fDockOffsetX, fDockOffsetY, sens, fInclinationOnHorizon, myDesklet->bIsHorizontal);
		
		//\____________________ On dessine les decorations dedans.
		cairo_save (pCairoContext);
		double fColor[4];
		int i;
		for (i = 0; i < 4; i ++)
		{
			fColor[i] = (g_fDeskletColorInside[i] * myDesklet->iGradationCount + g_fDeskletColor[i] * (10 - myDesklet->iGradationCount)) / 10;
		}
		cairo_set_source_rgba (pCairoContext, fColor[0], fColor[1], fColor[2], .8);
		cairo_fill_preserve (pCairoContext);
		cairo_restore (pCairoContext);
		
		//\____________________ On dessine le cadre.
		if (fLineWidth > 0)
		{
			cairo_set_line_width (pCairoContext, fLineWidth);
			cairo_set_source_rgba (pCairoContext, fColor[0], fColor[1], fColor[2], 1);
			cairo_stroke (pCairoContext);
		}
		cairo_restore (pCairoContext);
		
		if (pFirstDrawnElement != NULL)
		{
			do
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer != NULL)
				{
					cairo_save (pCairoContext);
					
					if (pIcon->fDrawY + pIcon->fHeight < myDesklet->iHeight / 2 && pIcon->fDrawX + pIcon->fWidth/2 > myDesklet->iWidth / 2)  // arriere-plan droite.
						cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, myDesklet->iWidth);
					
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_next_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
			
			do
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer != NULL)
				{
					cairo_save (pCairoContext);
					
					if (pIcon->fDrawY + pIcon->fHeight < myDesklet->iHeight / 2 && pIcon->fDrawX + pIcon->fWidth/2 < myDesklet->iWidth / 2)  // arriere-plan gauche.
						cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, myDesklet->iWidth);
					
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_previous_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
		}
		
		cairo_save (pCairoContext);
		myIcon->fDrawY = myDesklet->iHeight/2 - myIcon->fHeight;
		cairo_dock_render_one_icon_in_desklet (myIcon, pCairoContext, TRUE, FALSE, myDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		if (pFirstDrawnElement != NULL)
		{
			do
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer != NULL)
				{
					cairo_save (pCairoContext);
					
					if (pIcon->fDrawY + pIcon->fHeight > myDesklet->iHeight / 2 && pIcon->fDrawX + pIcon->fWidth/2 > myDesklet->iWidth / 2)  // avant-plan droite.
						cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, myDesklet->iWidth);
					
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_next_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
			
			do
			{
				pIcon = ic->data;
				if (pIcon->pIconBuffer != NULL)
				{
					cairo_save (pCairoContext);
					
					if (pIcon->fDrawY + pIcon->fHeight > myDesklet->iHeight / 2 && pIcon->fDrawX + pIcon->fWidth/2 < myDesklet->iWidth / 2)  // avant-plan gauche.
						cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, TRUE, TRUE, myDesklet->iWidth);
					
					cairo_restore (pCairoContext);
				}
				ic = cairo_dock_get_previous_element (ic, myData.pDeskletIconList);
			}
			while (ic != pFirstDrawnElement);
		}
	}
	else
	{
		cd_debug (" icone en (%.2f;%.2f)", myIcon->fDrawX, myIcon->fDrawY);
		cairo_save (pCairoContext);
		cairo_dock_render_one_icon_in_desklet (myIcon, pCairoContext, FALSE, FALSE, myDesklet->iWidth);
		cairo_restore (pCairoContext);
		
		int iNumIcon=0, iNbIcons = g_list_length (myData.pDeskletIconList);
		cd_debug ("%d icones a dessiner", iNbIcons);
		double fTheta = G_PI/2, fDeltaTheta = 2 * G_PI / iNbIcons;
		
		double a = MAX (myIcon->fWidth, myIcon->fHeight)/2 + .1*myDesklet->iWidth, b = MIN (myIcon->fWidth, myIcon->fHeight)/2 + .1*myDesklet->iHeight;
		double c = sqrt (a * a - b * b);
		double e = c / a;
		gboolean bFlip = (myIcon->fHeight > myIcon->fWidth);
		double fRadius;
		Icon *pIcon;
		GList *pElement;
		for (pElement = myData.pDeskletIconList; pElement != NULL; pElement = pElement->next)
		{
			pIcon = pElement->data;
			if (pIcon->pIconBuffer != NULL)
			{
				cairo_save (pCairoContext);
				
				fRadius = (bFlip ? sqrt (b * b / (1 - e * e * cos (G_PI/2-fTheta) * cos (G_PI/2-fTheta))) : sqrt (b * b / (1 - e * e * cos (fTheta) * cos (fTheta))));
				pIcon->fDrawX = myIcon->fDrawX + myIcon->fWidth / 2 + fRadius * cos (fTheta)-pIcon->fWidth/2;
				pIcon->fDrawY = myIcon->fDrawY + myIcon->fHeight / 2 + fRadius * sin (fTheta)-pIcon->fHeight/2;
				cairo_dock_render_one_icon_in_desklet (pIcon, pCairoContext, FALSE, TRUE, myDesklet->iWidth);
				
				cairo_restore (pCairoContext);
			}
			fTheta += fDeltaTheta;
		}
	}
}
gboolean on_scroll_desklet (GtkWidget* pWidget,
			GdkEventScroll* pScroll,
			CairoDockDesklet *pDesklet)
{
	if (myData.pFirstDrawnElement != NULL)
	{
		myData.pFirstDrawnElement = cairo_dock_get_next_element (myData.pFirstDrawnElement, myData.pDeskletIconList);
		gtk_widget_queue_draw (pDesklet->pWidget);
	}
}
CD_APPLET_INIT_BEGIN (erreur)
	if (!g_thread_supported ())
		g_thread_init (NULL);
	
	if (myDesklet != NULL)
	{
		if (myConfig.bDesklet3D)
		{
			myIcon->fWidth = MAX (1, MIN (myDesklet->iWidth, myDesklet->iHeight) * WEATHER_RATIO_ICON_DESKLET);
			myIcon->fHeight = myIcon->fWidth;
		}
		else
		{
			myIcon->fWidth = MAX (1, (myDesklet->iWidth - g_iDockRadius) * WEATHER_RATIO_ICON_DESKLET);
			myIcon->fHeight = MAX (1, (myDesklet->iHeight - g_iDockRadius) * WEATHER_RATIO_ICON_DESKLET);
		}
		myIcon->fDrawX = (myDesklet->iWidth - myIcon->fWidth) / 2;
		myIcon->fDrawY = (myDesklet->iHeight - myIcon->fHeight) / 2;
		myIcon->fScale = 1.;
		myIcon->fAlpha = 1.;
		myIcon->fWidthFactor = 1.;
		myIcon->fHeightFactor = 1.;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = cd_weather_draw_in_desklet;
		g_signal_connect (G_OBJECT (myDesklet->pWidget),
			"scroll-event",
			G_CALLBACK (on_scroll_desklet),
			myDesklet);
	}
	
	cd_weather_launch_measure ();
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	g_source_remove (myData.iSidTimer);
	myData.iSidTimer = 0;
	
	//\_________________ On libere toutes nos ressources.
	reset_data ();
	reset_config ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL)
	{
		if (myConfig.bDesklet3D)
		{
			myIcon->fWidth = MAX (1, MIN (myDesklet->iWidth, myDesklet->iHeight) * WEATHER_RATIO_ICON_DESKLET);
			myIcon->fHeight = myIcon->fWidth;
		}
		else
		{
			myIcon->fWidth = MAX (1, (myDesklet->iWidth - g_iDockRadius) * WEATHER_RATIO_ICON_DESKLET);
			myIcon->fHeight = MAX (1, (myDesklet->iHeight - g_iDockRadius) * WEATHER_RATIO_ICON_DESKLET);
		}
		myIcon->fDrawX = (myDesklet->iWidth - myIcon->fWidth) / 2;
		myIcon->fDrawY = (myDesklet->iHeight - myIcon->fHeight) / 2;
		myIcon->fScale = 1.;
		myIcon->fAlpha = 1.;
		myIcon->fWidthFactor = 1.;
		myIcon->fHeightFactor = 1.;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = cd_weather_draw_in_desklet;
	}
	g_return_val_if_fail (myConfig.cLocationCode != NULL, FALSE);
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		g_source_remove (myData.iSidTimer);
		myData.iSidTimer = 0;
		
		reset_data ();
		
		cd_weather_launch_measure ();
	}
	else if (myDesklet != NULL)
	{
		GList* ic;
		Icon *icon;
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
		for (ic = myData.pDeskletIconList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (myConfig.bDesklet3D)
			{
				icon->fWidth = 0;
				icon->fHeight = 0;
			}
			else
			{
				icon->fWidth = MAX (1, (myDesklet->iWidth - g_iDockRadius - myIcon->fWidth) / 2);
				icon->fHeight = MAX (1, (myDesklet->iHeight - g_iDockRadius - myIcon->fHeight) / 2);
			}
			cairo_dock_fill_icon_buffers (icon, pCairoContext, 1, CAIRO_DOCK_HORIZONTAL, myConfig.bDesklet3D);
		}
		cairo_destroy (pCairoContext);
	}
	else
	{
		// rien a faire, cairo-dock va recharger notre sous-dock.
	}
CD_APPLET_RELOAD_END
