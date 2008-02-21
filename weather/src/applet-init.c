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


CD_APPLET_DEFINITION ("weather", 1, 5, 1)


void cd_weather_draw_in_desklet (cairo_t *pCairoContext, gpointer data)
{
	g_print (" -> (%.2f;%.2f)\n", myIcon->fDrawX, myIcon->fDrawY);
	cairo_translate (pCairoContext, myIcon->fDrawX, myIcon->fDrawY);
	if (myIcon->pIconBuffer != NULL)
	{
		g_print (" -> (%.2f;%.2f)\n", myIcon->fDrawX, myIcon->fDrawY);
		cairo_set_source_surface (pCairoContext, myIcon->pIconBuffer, 0.0, 0.0);
		cairo_paint (pCairoContext);
	}
	
	///cairo_translate (pCairoContext, myIcon->fWidth / 2, myIcon->fHeight / 2);
	int iNumIcon=0, iNbIcons = g_list_length (myData.pDeskletIconList);
	g_print ("%d icones\n", iNbIcons);
	double fTheta = G_PI/2, fDeltaTheta = 2 * G_PI / iNbIcons;
	
	double a = 1.1*MAX (myIcon->fWidth, myIcon->fHeight)/2, b = 1.1*MIN (myIcon->fWidth, myIcon->fHeight)/2;
	double c = sqrt (a * a - b * b);
	double e = c / a;
	//b = 1.1*myIcon->fHeight / 2;
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
			/*cairo_move_to(pCairoContext, 
				(fRadius + 0*pIcon->fHeight) * cos (fTheta),
				(fRadius + 0*pIcon->fHeight) * sin (fTheta));
			cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, -pIcon->fWidth/2, -pIcon->fHeight/2);
			cairo_paint (pCairoContext);*/
			
			pIcon->fDrawX = myIcon->fWidth / 2 + fRadius * cos (fTheta);
			pIcon->fDrawY = myIcon->fHeight / 2 + fRadius * sin (fTheta);
			cairo_dock_render_one_icon (pIcon, pCairoContext, CAIRO_DOCK_HORIZONTAL, 1, 1, FALSE, TRUE, myDesklet->iWidth);
			
			cairo_restore (pCairoContext);
		}
		fTheta -= fDeltaTheta;
	}
}

CD_APPLET_INIT_BEGIN (erreur)
	if (!g_thread_supported ())
		g_thread_init (NULL);
	
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (1, (myDesklet->iWidth - g_iDockRadius) * .6);
		myIcon->fHeight = MAX (1, (myDesklet->iHeight - g_iDockRadius) * .6);
		myIcon->fDrawX = (myDesklet->iWidth - myIcon->fWidth) / 2;
		myIcon->fDrawY = (myDesklet->iHeight - myIcon->fHeight) / 2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = cd_weather_draw_in_desklet;
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
		myIcon->fWidth = MAX (1, (myDesklet->iWidth - g_iDockRadius) * .6);
		myIcon->fHeight = MAX (1, (myDesklet->iHeight - g_iDockRadius) * .6);
		myIcon->fDrawX = (myDesklet->iWidth - myIcon->fWidth) / 2;
		myIcon->fDrawY = (myDesklet->iHeight - myIcon->fHeight) / 2;
		myIcon->fScale = 1;
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
	else
	{
		// rien a faire, cairo-dock va recharger notre sous-dock.
	}
CD_APPLET_RELOAD_END
