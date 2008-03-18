/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-animation.h"
#include "applet-init.h"

AppletConfig myConfig;
AppletData myData;

static gboolean bDetached = FALSE;

CD_APPLET_DEFINITION ("Cairo-Penguin", 1, 5, 3, CAIRO_DOCK_CATEGORY_ACCESSORY)


CD_APPLET_INIT_BEGIN (erreur)
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU, CAIRO_DOCK_RUN_FIRST);
	cairo_dock_register_notification (CAIRO_DOCK_MIDDLE_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_MIDDLE_CLICK, CAIRO_DOCK_RUN_FIRST);
	
	penguin_load_theme (myConfig.cThemePath);
	
	penguin_start_animating_with_delay ();
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	g_source_remove (myData.iSidAnimation);
	myData.iSidAnimation = 0;
	
	if (myData.iSidRestartDelayed != 0)
	{
		g_source_remove (myData.iSidRestartDelayed);
		myData.iSidRestartDelayed = 0;
	}
	
	gulong iOnExposeCallbackID = g_signal_handler_find (G_OBJECT (myContainer->pWidget),
		G_SIGNAL_MATCH_FUNC,
		0,
		0,
		NULL,
		penguin_draw_on_dock,
		NULL);
	if (iOnExposeCallbackID > 0)
		g_signal_handler_disconnect (G_OBJECT (myContainer->pWidget), iOnExposeCallbackID);
	
	//\_________________ On libere toutes nos ressources.
	reset_data ();
	reset_config ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		g_source_remove (myData.iSidAnimation);
		myData.iSidAnimation = 0;
		if (myData.iSidRestartDelayed != 0)
		{
			g_source_remove (myData.iSidRestartDelayed);
			myData.iSidRestartDelayed = 0;
		}
		
		PenguinAnimation *pAnimation = penguin_get_current_animation ();
		if (pAnimation != NULL)
		{
			GdkRectangle area;
			area.x = (myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX;
			area.y = myDock->iCurrentHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight;
			area.width = pAnimation->iFrameWidth;
			area.height = pAnimation->iFrameHeight;
#ifdef HAVE_GLITZ
			if (myContainer->pDrawFormat && myContainer->pDrawFormat->doublebuffer)
				gtk_widget_queue_draw (myContainer->pWidget);
			else
#endif
			gdk_window_invalidate_rect (myContainer->pWidget->window, &area, FALSE);
		}
		
		reset_data ();
		
		penguin_load_theme (myConfig.cThemePath);
		
		if (myConfig.bFree)
		{
			cairo_dock_detach_icon_from_dock (myIcon, myDock, g_bUseSeparator);
			cairo_dock_update_dock_size (myDock);
		}
		else
		{
			cairo_dock_insert_icon_in_dock (myIcon, myDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
		}
		
		penguin_start_animating ();
	}
	else
	{
		
	}
CD_APPLET_RELOAD_END
