#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-desktops.h"
#include "applet-notifications.h"




CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myDock)
	{
		gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
		cairo_dock_show_hide_desktop (! bDesktopIsVisible);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_CLICK_BEGIN
	
	int iNumLine, iNumColumn;
	int iNumDesktop, iNumViewportX, iNumViewportY;
	
	if (myConfig.bCompactView && pClickedIcon == myIcon)
	{
		int iMouseX, iMouseY;
		iMouseX = myContainer->iMouseX - myIcon->fDrawX;
		iMouseY = myContainer->iMouseY - myIcon->fDrawY;
		if (! myContainer->bIsHorizontal)
		{
			double tmp = iMouseX;
			iMouseX = iMouseY;
			iMouseY = tmp;
		}
		
		if (iMouseX < 0)
			iMouseX = 0;
		if (iMouseY < 0)
			iMouseY = 0;
		if (iMouseX > myIcon->fWidth * myIcon->fScale)
			iMouseX = myIcon->fWidth * myIcon->fScale;
		if (iMouseY > myIcon->fHeight * myIcon->fScale)
			iMouseY = myIcon->fHeight * myIcon->fScale;
		iNumLine = (int) (iMouseY / (myIcon->fHeight * myIcon->fScale) * myData.switcher.iNbLines);
		iNumColumn = (int) (iMouseX / (myIcon->fWidth * myIcon->fScale) * myData.switcher.iNbColumns);
		cd_switcher_compute_desktop_from_coordinates (iNumLine, iNumColumn, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	}
	else if (pClickedIcon != NULL && pClickedIcon != myIcon)
	{
		int iIndex = pClickedIcon->fOrder;
		cd_switcher_compute_viewports_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (iNumDesktop != myData.switcher.iCurrentDesktop)
		cairo_dock_set_current_desktop (iNumDesktop);
	if (iNumViewportX != myData.switcher.iCurrentViewportX || iNumViewportY != myData.switcher.iCurrentViewportY)
		cairo_dock_set_current_viewport (iNumViewportX, iNumViewportY);
CD_APPLET_ON_CLICK_END


static void _cd_switcher_add_desktop (GtkMenuItem *menu_item, Icon *pIcon)
{
	cd_switcher_add_a_desktop ();
}
static void _cd_switcher_remove_last_desktop (GtkMenuItem *menu_item, Icon *pIcon)
{
	cd_switcher_remove_last_desktop ();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_IN_MENU (_("Add a desktop"), _cd_switcher_add_desktop, pSubMenu);
		CD_APPLET_ADD_IN_MENU (_("Remove last desktop"), _cd_switcher_remove_last_desktop, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


