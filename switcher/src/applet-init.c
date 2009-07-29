/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Cchumi & Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include "stdlib.h"
#include "string.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-desktops.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("switcher",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_DESKTOP,
	N_("This applet allows you to switch between your virtual desktops.\n"
	"It has 2 modes : compact (on 1 icon) and expanded (with a sub-dock).\n"
	"You can also quickly add/remove a desktop through the right-click menu.\n"
	"Scroll up/down on the main icon to go to the left/right desktop, middle-click to show the desktop"),
	"Cchumi")


CD_APPLET_INIT_BEGIN
	CD_APPLET_SET_STATIC_ICON;
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	cairo_dock_register_notification (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED,
		(CairoDockNotificationFunc) on_change_screen_geometry,
		CAIRO_DOCK_RUN_AFTER, myApplet);
	cairo_dock_register_notification (CAIRO_DOCK_DESKTOP_CHANGED,
		(CairoDockNotificationFunc) on_change_desktop,
		CAIRO_DOCK_RUN_AFTER, myApplet);/*Notifier d'un changement de bureau*/
	cairo_dock_register_notification (CAIRO_DOCK_WINDOW_CONFIGURED,
		(CairoDockNotificationFunc) on_window_configured,
		CAIRO_DOCK_RUN_AFTER, myApplet);	
	cairo_dock_register_notification (CAIRO_DOCK_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) on_change_active_window,
		CAIRO_DOCK_RUN_AFTER, myApplet);
	
	cd_switcher_update_from_screen_geometry ();
	
	//\___________________ On affiche le numero du bureau courant.
	if (myConfig.bDisplayNumDesk)
	{
		int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1);
	}
	
	//\___________________ Dans le cas ou l'applet demarre au chargement de la session, le nombre de bureaux peut etre incorrect.
	if (cairo_dock_is_loading ())
		myData.iSidAutoRefresh = g_timeout_add_seconds (2, cd_switcher_refresh_desktop_values, myApplet);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	if (myData.iSidRedrawMainIconIdle != 0)
	{
		g_source_remove (myData.iSidRedrawMainIconIdle);
	}
	if (myData.iSidAutoRefresh != 0)
	{
		g_source_remove (myData.iSidAutoRefresh);
	}
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	cairo_dock_remove_notification_func (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED,
		(CairoDockNotificationFunc) on_change_screen_geometry, myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_DESKTOP_CHANGED,
		(CairoDockNotificationFunc) on_change_desktop, myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_WINDOW_CONFIGURED,
		(CairoDockNotificationFunc) on_window_configured, myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) on_change_active_window, myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myData.iSidRedrawMainIconIdle != 0)
	{
		g_source_remove (myData.iSidRedrawMainIconIdle);
		myData.iSidRedrawMainIconIdle = 0;
	}
	
	if (myDesklet)
	{
		if (myConfig.bCompactView)
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
		else
		{
			gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", pConfig);
		}
	}
	///else if (myIcon->acName == NULL || *myIcon->acName == '\0')  // il faut un nom en cas de sous-dock.
	///	myIcon->acName = g_strdup (SWITCHER_DEFAULT_NAME);  // l'afficher ne nous interesse pas.
	
	cd_switcher_compute_nb_lines_and_columns ();
	
	cd_switcher_compute_desktop_coordinates (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY, &myData.switcher.iCurrentLine, &myData.switcher.iCurrentColumn);
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myConfig.bDisplayNumDesk)
		{
			int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1);
		}
		else
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
			
		cd_switcher_load_icons ();
	}
	else
	{
		if (! myConfig.bCompactView)
			cd_switcher_paint_icons ();
	}
	
	cd_switcher_draw_main_icon ();
CD_APPLET_RELOAD_END
