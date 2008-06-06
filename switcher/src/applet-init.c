
#include "stdlib.h"
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"
//#include "applet-draw.h"



AppletConfig myConfig;
AppletData myData;
extern cairo_surface_t *g_pDesktopBgSurface;

CD_APPLET_DEFINITION ("switcher", 1, 5, 4, CAIRO_DOCK_CATEGORY_DESKTOP)


static void _load_surfaces (void)
{

	GString *sImagePath = g_string_new ("");
	
/*Surface par défaut pour la vue simplifié*/

	if (myData.pSurface != NULL)
		cairo_surface_destroy (myData.pSurface);
	if (myConfig.cDefaultIcon != NULL)
	{

		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cDefaultIcon);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);

		g_free (cUserImagePath);
		cd_message ("ok default");
	}
	else
	{
	
		g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
		cd_message ("ok default 2");
	}

/*Surface par défaut pour la vue mode dock*/

	if (myData.pSurfaceSDock != NULL)
		cairo_surface_destroy (myData.pSurfaceSDock);
	if (myConfig.cDefaultSDockIcon != NULL)
	{

		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cDefaultSDockIcon);
		myData.pSurfaceSDock = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);

		g_free (cUserImagePath);
		cd_message ("ok default");
	}
	else
	{
	
		g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurfaceSDock = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
		cd_message ("ok default 2");
	}
	
/*Surface en cas de désagrément. Je n'en ai pas encore vu l'utilité*/

	if (myData.pBrokenSurface != NULL)
		cairo_surface_destroy (myData.pBrokenSurface);
	if (myConfig.cBrokenIcon != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cBrokenIcon);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/broken.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pBrokenSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
		cd_message ("ok broken");
	}
	
	g_string_free (sImagePath, TRUE);
}


CD_APPLET_INIT_BEGIN (erreur)
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	cairo_dock_register_notification (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, (CairoDockNotificationFunc) cd_switcher_launch_measure, CAIRO_DOCK_RUN_AFTER);/*Notifier de la gémotrie de bureau changé*/
	cairo_dock_register_notification (CAIRO_DOCK_DESKTOP_CHANGED, (CairoDockNotificationFunc) cd_switcher_launch_measure, CAIRO_DOCK_RUN_AFTER);/*Notifier d'un changement de bureau*/

if (g_pDesktopBgSurface == NULL)
{
printf("Background null Load Function /n");
cairo_dock_load_desktop_background_surface ();

}	
	
	if (myDesklet != NULL)
	{
		if (myConfig.bCurrentView)
		{
			
			cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
			myDrawContext = cairo_create (myIcon->pIconBuffer);
			CD_APPLET_REDRAW_MY_ICON
		}
		else
		{
			
			cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
			myDrawContext = cairo_create (myIcon->pIconBuffer);
			CD_APPLET_REDRAW_MY_ICON
		}
	}

	


_load_surfaces();
cd_switcher_launch_measure();


/*Ancienne fonction avec timer*/
//myData.LoadAfterCompiz = g_timeout_add (2000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);


CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	cairo_dock_remove_notification_func (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, (CairoDockNotificationFunc) cd_switcher_launch_measure);
	cairo_dock_remove_notification_func (CAIRO_DOCK_DESKTOP_CHANGED, (CairoDockNotificationFunc) cd_switcher_launch_measure);

/* Fonction plus utile car j'ai enlevé le timer */	
	/*if (myData.LoadAfterCompiz != 0)
	{
		g_source_remove (myData.LoadAfterCompiz);
		myData.LoadAfterCompiz = 0;
	}*/
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN

/* Fonction plus utile car j'ai enlevé le timer */

	/*if (myData.LoadAfterCompiz != 0)
	{
		g_source_remove (myData.LoadAfterCompiz);
		myData.LoadAfterCompiz = 0;
	}*/
	
	reset_data ();  /// quelle violence !
	if (myIcon->acName == NULL || *myIcon->acName == '\0')
		myIcon->acName = g_strdup (SWITCHER_DEFAULT_NAME);
	
	
	if (myDesklet != NULL)
	{
		if (myConfig.bCurrentView)
		{
			cd_message ("test");
			cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
			myDrawContext = cairo_create (myIcon->pIconBuffer);
			CD_APPLET_REDRAW_MY_ICON
		}
		else
		{
			
			cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
			myDrawContext = cairo_create (myIcon->pIconBuffer);
			CD_APPLET_REDRAW_MY_ICON
			
		}
	}

_load_surfaces();
cd_switcher_launch_measure();

/*Ancienne fonction avec timer*/
//myData.LoadAfterCompiz = g_timeout_add (2000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);

CD_APPLET_RELOAD_END
