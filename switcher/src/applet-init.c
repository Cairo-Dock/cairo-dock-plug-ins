
#include "stdlib.h"
#include "string.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-load-icons.h"
#include "applet-desktops.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("switcher", 1, 6, 2, CAIRO_DOCK_CATEGORY_DESKTOP)


static gboolean on_change_desktop (gpointer *data, CairoDockModuleInstance *myApplet)
{
	cd_message ("");
	int iPreviousIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	cd_switcher_get_current_desktop ();
	int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
	
	
	if (myConfig.bDisplayNumDesk)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1)
	}
	
	if (myConfig.bCompactView)
	{
		cd_switcher_draw_main_icon ();
	}
	else
	{
		CairoContainer *pContainer = (myDock ? CAIRO_CONTAINER (myIcon->pSubDock) : myContainer);
		g_return_val_if_fail (pContainer != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
		
		if (myDock && myConfig.bDisplayNumDesk)
			CD_APPLET_REDRAW_MY_ICON
		
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
static gboolean on_change_screen_geometry (gpointer *data, CairoDockModuleInstance *myApplet)
{
	cd_message ("");
	cd_switcher_compute_nb_lines_and_columns ();
	cd_switcher_get_current_desktop ();
	cd_switcher_load_icons ();
	cd_switcher_load_icons ();
	cd_switcher_draw_main_icon ();
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

CD_APPLET_INIT_BEGIN
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	cairo_dock_register_notification (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, (CairoDockNotificationFunc) on_change_screen_geometry, CAIRO_DOCK_RUN_AFTER, myApplet);/*Notifier de la geometrie de bureau changée*/
	cairo_dock_register_notification (CAIRO_DOCK_DESKTOP_CHANGED, (CairoDockNotificationFunc) on_change_desktop, CAIRO_DOCK_RUN_AFTER, myApplet);/*Notifier d'un changement de bureau*/
	cairo_dock_register_notification (CAIRO_DOCK_WINDOW_CONFIGURED, (CairoDockNotificationFunc) cd_switcher_draw_main_icon, CAIRO_DOCK_RUN_AFTER, myApplet);	
	cairo_dock_register_notification (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_switcher_draw_main_icon, CAIRO_DOCK_RUN_AFTER, myApplet);	
	
	//\___________________ On calcule la geometrie de l'icone en mode compact.
	cd_switcher_compute_nb_lines_and_columns ();
	
	//\___________________ On recupere le bureau courant et sa position sur la grille.
	cd_switcher_get_current_desktop ();
	
	//\___________________ On charge le bon nombre d'icones dans le sous-dock ou le desklet.
	cd_switcher_load_icons ();
	
	//\___________________ On dessine l'icone principale.
	cd_switcher_draw_main_icon ();
	
	//\___________________ On affiche le numero du bureau courant.
	if (myConfig.bDisplayNumDesk)
	{
		int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1)
	}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	cairo_dock_remove_notification_func (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, (CairoDockNotificationFunc) on_change_screen_geometry, myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_DESKTOP_CHANGED, (CairoDockNotificationFunc) on_change_desktop, myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_WINDOW_CONFIGURED, (CairoDockNotificationFunc) cd_switcher_draw_main_icon, myApplet);
	cairo_dock_remove_notification_func (CAIRO_DOCK_WINDOW_ACTIVATED, (CairoDockNotificationFunc) cd_switcher_draw_main_icon, myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myIcon->acName == NULL || *myIcon->acName == '\0')
		myIcon->acName = g_strdup (SWITCHER_DEFAULT_NAME);
	
	if (myDesklet != NULL)
	{
		if (myConfig.bCompactView)
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple")
		}
		else
		{
			gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", pConfig);
		}
	}
	
	cd_switcher_compute_nb_lines_and_columns ();
	
	cd_switcher_compute_desktop_coordinates (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY, &myData.switcher.iCurrentLine, &myData.switcher.iCurrentColumn);
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myConfig.bDisplayNumDesk)
		{
			int iIndex = cd_switcher_compute_index (myData.switcher.iCurrentDesktop, myData.switcher.iCurrentViewportX, myData.switcher.iCurrentViewportY);
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", iIndex+1)
		}
		else
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
			
		cd_switcher_load_icons ();
	}
	else
	{
		cd_switcher_paint_icons ();
	}
	
	cd_switcher_draw_main_icon ();
CD_APPLET_RELOAD_END
