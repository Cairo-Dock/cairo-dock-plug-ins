#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_ABOUT (_D("This is the switcher applet\n made by pierre for Cairo-Dock"))

CD_APPLET_ON_CLICK_BEGIN
myData.loadaftercompiz = g_timeout_add (1000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);
//gboolean g_bAppliOnCurrentDesktopOnly;
cd_debug (" move to A %d", CAIRO_DOCK_DESKTOP_CHANGED);
//cd_debug (" move to %d", g_bAppliOnCurrentDesktopOnly);
	if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_DOCK_CONTAINER (myIcon->pSubDock))  // on a clique sur une icone du sous-dock.
	{
		cd_debug (" clic sur %s", pClickedIcon->acName);
		//cd_weather_show_forecast_dialog (pClickedIcon);
int iNbViewportX, iNbViewportY;
cairo_dock_get_nb_viewports (&iNbViewportX, &iNbViewportY);
int iDesktopViewportX = atoi (pClickedIcon->cQuickInfo);
int iDesktopViewportY = iNbViewportY;
//int numclicked = atoi (pClickedIcon->acName);
cairo_dock_set_current_viewport (iDesktopViewportX, iDesktopViewportY);

	}
	else if (myDesklet != NULL && pClickedContainer == myContainer && pClickedIcon != NULL)  // on a clique sur une icone du desklet.
	{
		if (pClickedIcon == myIcon)
			myData.loadaftercompiz = g_timeout_add (1000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);
		else
			myData.loadaftercompiz = g_timeout_add (1000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;

CD_APPLET_ON_CLICK_END

static void _cd_switcher_reload (GtkMenuItem *menu_item, gpointer *data)
{
	g_source_remove (myData.loadaftercompiz);
	myData.loadaftercompiz = 0;
	
	cd_switcher_launch_measure ();  // asynchrone
}


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Switchers", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_("Reload now"), _cd_switcher_reload, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN

	gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
	cairo_dock_show_hide_desktop (! bDesktopIsVisible);
CD_APPLET_ON_MIDDLE_CLICK_END
