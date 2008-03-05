/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-draw.h"
#include "applet-init.h"

AppletConfig myConfig;
AppletData myData;

CD_APPLET_DEFINITION ("weather", 1, 5, 1)


static gboolean _cd_weather_rotate (gpointer data)
{
	myData.iRotationCount += (myData.iRotationDirection == GDK_SCROLL_UP ? 1 : -1);
	myData.fRotationAngle += (myData.iRotationDirection == GDK_SCROLL_UP ? 1 : -1) * 2 * G_PI / myData.iNbIcons / 10;
	if (myData.fRotationAngle >= 2*G_PI)
		myData.fRotationAngle -= 2*G_PI;
	else if (myData.fRotationAngle < 0)
		myData.fRotationAngle += 2*G_PI;
	gtk_widget_queue_draw (myDesklet->pWidget);
	if (abs (myData.iRotationCount) >= 10 || myData.iRotationCount == 0)
	{
		myData.iRotationCount = 0;
		myData.iSidRotation = 0;
		return FALSE;
	}
	else
		return TRUE;
}

static gboolean on_scroll_desklet (GtkWidget* pWidget,
	GdkEventScroll* pScroll,
	CairoDockDesklet *pDesklet)
{
	if (myData.pDeskletIconList != NULL && (pScroll->direction == GDK_SCROLL_DOWN || pScroll->direction == GDK_SCROLL_UP))
	{
		if (myData.iSidRotation == 0)
		{
			myData.iRotationDirection = pScroll->direction;
			myData.iSidRotation = g_timeout_add (50, (GSourceFunc) _cd_weather_rotate, (gpointer) NULL);
		}
		else
		{
			if (pScroll->direction == myData.iRotationDirection)
			{
				myData.iRotationCount = 0;
			}
			else
			{
				myData.iRotationDirection = pScroll->direction;
			}
		}
		_cd_weather_rotate (NULL);
	}
}
CD_APPLET_INIT_BEGIN (erreur)
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
		myIcon->fDrawY = (myDesklet->iHeight - myIcon->fHeight) / 2 + (myConfig.bDesklet3D ? g_iLabelSize : 0);
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
	else if (myIcon->acName == NULL || *myIcon->acName == '\0')
		myIcon->acName = g_strdup (WEATHER_DEFAULT_NAME);
	
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
		myIcon->fDrawY = (myDesklet->iHeight - myIcon->fHeight) / 2 + (myConfig.bDesklet3D ? g_iLabelSize : 0);
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
		if (myIcon->acName == NULL || *myIcon->acName == '\0')
			myIcon->acName = g_strdup (WEATHER_DEFAULT_NAME);
		
		cd_weather_launch_measure ();  // asynchrone
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
				icon->fWidth = MAX (1, .2 * myDesklet->iWidth - g_iLabelSize);
				icon->fHeight = MAX (1, .2 * myDesklet->iHeight - g_iLabelSize);
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
