
#include "stdlib.h"
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-load-icons.h"
#include "applet-draw.h"



AppletConfig myConfig;
AppletData myData;


CD_APPLET_DEFINITION ("switcher", 1, 5, 4, CAIRO_DOCK_CATEGORY_DESKTOP)

static void _load_surfaces (void)
{
	GString *sImagePath = g_string_new ("");
	
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
cairo_dock_register_notification (CAIRO_DOCK_SCREEN_GEOMETRY_ALTERED, (CairoDockNotificationFunc) cd_switcher_launch_measure, CAIRO_DOCK_RUN_FIRST);

cairo_dock_register_notification (CAIRO_DOCK_DESKTOP_CHANGED, (CairoDockNotificationFunc) cd_switcher_launch_measure, CAIRO_DOCK_RUN_AFTER);



 if (myDesklet != NULL)
	{
		if (myConfig.bCurrentView)
		{
		cd_message ("test");
cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
	myDrawContext = cairo_create (myIcon->pIconBuffer);
CD_APPLET_REDRAW_MY_ICON
		}
else	{
cd_message ("test2");
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
CD_APPLET_REDRAW_MY_ICON
	}

	}

_load_surfaces();

//myData.LoadAfterCompiz = g_timeout_add (2000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);

//GdkWindow * root;
//	GdkPixbuf* img;

//GdkScreen	*screen = gdk_screen_get_default();
//root = gdk_screen_get_root_window( screen );
//gint x = gdk_screen_get_width  (screen);
//gint y = gdk_screen_get_height  (screen);

//printf ("X%d \n",x);
//printf ("Y%d \n",y);
      //gdk_drawable_get_size (GDK_DRAWABLE (window), 1024, 768);
      //gdk_window_get_origin (window, &x, &y);

  //GdkPixbuf    *screenshot = gdk_pixbuf_get_from_drawable (NULL, root,
     //                                    NULL,
    //                                     0,0, 0, 0,
     //                                    x, y);
//img = create_image (screenshot);
//GdkPixbuf    *screenshot2 = gdk_pixbuf_scale_simple      (screenshot,
  //                                                       640,
   //                                                      480,
   //                                                     GDK_INTERP_BILINEAR);
//gdk_pixbuf_save(screenshot2, "/tmp/essai.png", "png",  NULL);
                //      "compression", "9",
                     // NULL);
// create a new pixbuf from the root window
//gtk_widget_show_all (widget);
	cd_switcher_launch_measure();
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	

		g_source_remove (myData.LoadAfterCompiz);
	myData.LoadAfterCompiz = 0;
CD_APPLET_STOP_END

CD_APPLET_RELOAD_BEGIN

		g_source_remove (myData.LoadAfterCompiz);
		myData.LoadAfterCompiz = 0;
		
		reset_data ();
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
else	{
cd_message ("test2");
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}

	}

_load_surfaces();

cd_switcher_launch_measure();  // asynchrone

CD_APPLET_RELOAD_END
