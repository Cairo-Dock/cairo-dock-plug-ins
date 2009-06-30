#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-desktops.h"
#include "applet-draw.h"
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
		if (myDesklet)
		{
			cairo_dock_get_coords_on_3D_desklet (myDesklet, &iMouseX, &iMouseY);
			iMouseX -= myIcon->fDrawX;
			iMouseY -= myIcon->fDrawY;
		}
		else
		{
			iMouseX = myContainer->iMouseX - myIcon->fDrawX;
			iMouseY = myContainer->iMouseY - myIcon->fDrawY;
		}
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


CD_APPLET_ON_SCROLL_BEGIN  // Merci ChangFu !
  int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
  int iNumDesktop, iNumViewportX, iNumViewportY;
  cd_debug ("Switcher: current %d", iIndex);
  if (CD_APPLET_SCROLL_DOWN) {
    iIndex++;
    if (iIndex >= myData.switcher.iNbViewportTotal) {
      iIndex = 0;
    }
    cd_debug ("Switcher: switching to %d", iIndex);
    cd_switcher_compute_viewports_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
  }
  else if (CD_APPLET_SCROLL_UP) {
    iIndex = iIndex - 1;
    if (iIndex < 0) {
      iIndex = myData.switcher.iNbViewportTotal - 1;
    }
    cd_debug ("Switcher: switching to %d", iIndex);
    cd_switcher_compute_viewports_from_index (iIndex, &iNumDesktop, &iNumViewportX, &iNumViewportY);
  }
  else
  	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
  	
	if (iNumDesktop != myData.switcher.iCurrentDesktop)
		cairo_dock_set_current_desktop (iNumDesktop);
	if (iNumViewportX != myData.switcher.iCurrentViewportX || iNumViewportY != myData.switcher.iCurrentViewportY)
		cairo_dock_set_current_viewport (iNumViewportX, iNumViewportY);
CD_APPLET_ON_SCROLL_END


static void _cd_switcher_add_desktop (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_switcher_add_a_desktop ();
}
static void _cd_switcher_remove_last_desktop (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_switcher_remove_last_desktop ();
}
static void _cd_switcher_refresh (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_switcher_refresh_desktop_values (myApplet);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (_("Add a desktop"),
			GTK_STOCK_ADD,
			_cd_switcher_add_desktop,
			pSubMenu);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (_("Remove last desktop"),
			GTK_STOCK_REMOVE,
			_cd_switcher_remove_last_desktop,
			pSubMenu);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (_("Refresh"),
			GTK_STOCK_REFRESH,
			_cd_switcher_refresh,
			pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END



static gboolean _cd_switcher_redraw_main_icon_idle (CairoDockModuleInstance *myApplet)
{
	cd_switcher_draw_main_icon ();
	myData.iSidRedrawMainIconIdle = 0;
	return FALSE;
}
static void _cd_switcher_queue_draw (CairoDockModuleInstance *myApplet)
{
	if (myData.iSidRedrawMainIconIdle == 0)
	{
		myData.iSidRedrawMainIconIdle = g_idle_add ((GSourceFunc) _cd_switcher_redraw_main_icon_idle, myApplet);
	}
}

gboolean on_change_active_window (CairoDockModuleInstance *myApplet, Window *XActiveWindow)
{
	_cd_switcher_queue_draw (myApplet);
	//cd_switcher_draw_main_icon ();
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean on_change_desktop (CairoDockModuleInstance *myApplet)
{
	cd_debug ("");
	int iPreviousIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	cd_switcher_get_current_desktop ();
	int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	if (myConfig.bDisplayNumDesk)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1);
	}
	
	if (myConfig.bCompactView)
	{
		_cd_switcher_queue_draw (myApplet);
		//cd_switcher_draw_main_icon ();
	}
	else
	{
		CairoContainer *pContainer = (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer);
		g_return_val_if_fail (pContainer != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
		
		if (myDock && myConfig.bDisplayNumDesk)
			CD_APPLET_REDRAW_MY_ICON;
		
		// On redessine les 2 icones du sous-dock impactees.
		GList *pIconList = (myDock ? myIcon->pSubDock->icons : myDesklet->icons);
		Icon *icon;
		GList *ic;
		for (ic = pIconList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->fOrder == iPreviousIndex)  // l'ancienne icone du bureau courant.
			{
				cairo_dock_set_icon_name_full (myDrawContext, icon, pContainer, "%s %d", D_("Desktop"), iPreviousIndex+1);
				icon->bHasIndicator = FALSE;
				icon->fAlpha = 1.;
				if (myDock)
					cairo_dock_redraw_my_icon (icon, pContainer);
			}
			if (icon->fOrder == iIndex)  // c'est l'icone du bureau courant.
			{
				cairo_dock_set_icon_name_full (myDrawContext, icon, pContainer, "%s %d", D_("Current"), iIndex+1);
				icon->bHasIndicator = TRUE;
				icon->fAlpha = .7;
				if (myDock)
					cairo_dock_redraw_my_icon (icon, pContainer);
			}
		}
		if (myDesklet)
			gtk_widget_queue_draw (myDesklet->pWidget);
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean on_change_screen_geometry (CairoDockModuleInstance *myApplet)
{
	cd_debug ("");
	cd_switcher_update_from_screen_geometry ();
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean on_window_configured (CairoDockModuleInstance *myApplet, XConfigureEvent *xconfigure)
{
	cd_debug ("");
	_cd_switcher_queue_draw (myApplet);
	//cd_switcher_draw_main_icon ();
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
