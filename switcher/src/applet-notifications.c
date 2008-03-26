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


CD_APPLET_ABOUT (_D("This is the switcher applet\n made by Cchumi for Cairo-Dock"))

static void _cd_switcher_reload (GtkMenuItem *menu_item, gpointer *data)
{
	g_source_remove (myData.loadaftercompiz);
	myData.loadaftercompiz = 0;
	
	cd_switcher_launch_measure ();  // asynchrone
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myDock)
	{
		gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
		g_print ("bDesktopIsVisible : %d\n", bDesktopIsVisible);
		cairo_dock_show_hide_desktop (! bDesktopIsVisible);
	}
CD_APPLET_ON_MIDDLE_CLICK_END

CD_APPLET_ON_CLICK_BEGIN
	if (pClickedIcon == myIcon)
		{	

cairo_dock_get_nb_viewports (&myData.switcher.iNbViewportX, &myData.switcher.iNbViewportY);

myData.switcher.iDesktopViewportX = myData.switcher.ScreenCurrentNums + 1;
myData.switcher.iDesktopViewportY = myData.switcher.iNbViewportY;
CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%d", myData.switcher.ScreenCurrentNums)
cd_debug (" Num Current Desks Clic  %d", myData.switcher.ScreenCurrentNums);
cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
cd_debug (" icone X %f", myIcon->fDrawX);
cd_debug (" icone Y %f", myIcon->fDrawY);
cd_debug (" Dock X %f", myDock->iMouseX);
cd_debug (" Dock Y %f", myDock->iMouseY);
cd_debug (" Dock Y %f", myDock->iCurrentHeight);
cd_debug (" Dock Y %f", myDock->iCurrentWidth);
//cd_switcher_launch_measure ();
myData.loadaftercompiz = g_timeout_add (500, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);
}
else
	if (myDock != NULL && myIcon->pSubDock != NULL && pClickedContainer == CAIRO_DOCK_CONTAINER (myIcon->pSubDock))  // on a clique sur une icone du sous-dock.
	{

		cd_debug (" clic sur %s", pClickedIcon->acName);
myData.switcher.iDesktopViewportX = atoi (pClickedIcon->cQuickInfo);

cairo_dock_set_current_viewport (myData.switcher.iDesktopViewportX, myData.switcher.iDesktopViewportY);
cd_switcher_launch_measure ();

	}
	
else

		return CAIRO_DOCK_LET_PASS_NOTIFICATION;

CD_APPLET_ON_CLICK_END	

CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("switcher", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_("Reload now"), _cd_switcher_reload, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
