
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-mixer.h"
#include "applet-draw.h"
#include "applet-init.h"

AppletConfig myConfig;
AppletData myData;

CD_APPLET_DEFINITION ("AlsaMixer", 1, 5, 3, CAIRO_DOCK_CATEGORY_CONTROLER)


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
	}
	else
	{
		g_string_printf (sImagePath, "%s/default.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	/*if (myData.pBrokenSurface != NULL)
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
	}*/
	
	if (myData.pMuteSurface != NULL)
		cairo_surface_destroy (myData.pMuteSurface);
	if (myConfig.cMuteIcon != NULL)
	{
		gchar *cUserImagePath = cairo_dock_generate_file_path (myConfig.cMuteIcon);
		myData.pMuteSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (cUserImagePath);
		g_free (cUserImagePath);
	}
	else
	{
		g_string_printf (sImagePath, "%s/mute.svg", MY_APPLET_SHARE_DATA_DIR);
		myData.pMuteSurface = CD_APPLET_LOAD_SURFACE_FOR_MY_APPLET (sImagePath->str);
	}
	
	g_string_free (sImagePath, TRUE);
}


static gboolean _cd_mixer_on_enter (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	gpointer data)
{
	if (myDesklet)
	{
		gtk_widget_show (myData.pScale);
	}
}
gboolean _cd_mixer_on_leave (GtkWidget* pWidget,
	GdkEventCrossing* pEvent,
	gpointer data)
{
	if (myDesklet)
	{
		if (! myDesklet->bInside)
			gtk_widget_hide (myData.pScale);
	}
}

CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (MAX (1, g_iDockRadius), MIN (myDesklet->iWidth, myDesklet->iHeight) - 0*g_iDockRadius - 15);
		myIcon->fHeight = myIcon->fWidth;
		myIcon->fDrawX = 0*g_iDockRadius/2;
		myIcon->fDrawY = myDesklet->iHeight - myIcon->fHeight + 0*g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		/*myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;*/
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		if (myConfig.bHideScaleOnLeave)
		{
			g_signal_connect (G_OBJECT (myDesklet->pWidget),
				"enter-notify-event",
				G_CALLBACK (_cd_mixer_on_enter),
				NULL);
			g_signal_connect (G_OBJECT (myDesklet->pWidget),
				"leave-notify-event",
				G_CALLBACK (_cd_mixer_on_leave),
				NULL);
		}
	}
	
	_load_surfaces ();
	
	mixer_init (myConfig.card_id);
	
	mixer_write_elements_list (CD_APPLET_MY_CONF_FILE, CD_APPLET_MY_KEY_FILE);
	mixer_get_controlled_element ();
	
	if (myData.pControledElement == NULL)
	{
		gchar *cImagePath = (myConfig.cBrokenIcon != NULL ? cairo_dock_generate_file_path (myConfig.cBrokenIcon) : g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/broken.svg", NULL));
		CD_APPLET_SET_IMAGE_ON_MY_ICON (cImagePath)
		g_free (cImagePath);
		//CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pBrokenSurface)
	}
	else
	{
		if (myDesklet)
		{
			GtkWidget *box = gtk_hbox_new (FALSE, 0);
			myData.pScale = mixer_build_widget (FALSE);
			gtk_box_pack_end (GTK_BOX (box), myData.pScale, FALSE, FALSE, 0);
			gtk_widget_show_all (box);
			gtk_container_add (GTK_CONTAINER (myDesklet->pWidget), box);
			
			g_signal_connect (G_OBJECT (myDesklet->pWidget),
				"enter-notify-event",
				G_CALLBACK (_cd_mixer_on_enter),
				NULL);
			g_signal_connect (G_OBJECT (myDesklet->pWidget),
				"leave-notify-event",
				G_CALLBACK (_cd_mixer_on_leave),
				NULL);
		}
		
		mixer_element_update_with_event (myData.pControledElement, 1);
		myData.iSidCheckVolume = g_timeout_add (1000, (GSourceFunc) mixer_check_events, (gpointer) NULL);
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	
	cd_keybinder_bind (myConfig.cShortcut, (CDBindkeyHandler) mixer_on_keybinding_pull, (gpointer)NULL);
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	//\_________________ On stoppe le timer.
	if (myData.iSidCheckVolume != 0)
	{
		g_source_remove (myData.iSidCheckVolume);
		myData.iSidCheckVolume = 0;
	}
	
	//\_________________ On libere toutes nos ressources.
	reset_data ();
	reset_config ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet != NULL)
	{
		myIcon->fWidth = MAX (MAX (1, g_iDockRadius), MIN (myDesklet->iWidth, myDesklet->iHeight) - 0*g_iDockRadius - 15);
		myIcon->fHeight = myIcon->fWidth;
		myIcon->fDrawX = 0*g_iDockRadius/2;
		myIcon->fDrawY = myDesklet->iHeight - myIcon->fHeight + 0*g_iDockRadius/2;
		myIcon->fScale = 1;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		/*myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;*/
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	//\_______________ On recharge les donnees qui ont pu changer.
	_load_surfaces ();
	
	//\_______________ On recharge le mixer si necessaire.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myData.iSidCheckVolume != 0)
		{
			g_source_remove (myData.iSidCheckVolume);
			myData.iSidCheckVolume = 0;
		}
		
		mixer_stop ();
		g_free (myData.cErrorMessage);
		myData.cErrorMessage = NULL;
		g_free (myData.mixer_card_name);
		myData.mixer_card_name = NULL;
		g_free (myData.mixer_device_name);
		myData.mixer_device_name= NULL;
		
		if (myConfig.iVolumeDisplay != VOLUME_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
		
		mixer_init (myConfig.card_id);
		mixer_write_elements_list (CD_APPLET_MY_CONF_FILE, CD_APPLET_MY_KEY_FILE);
		mixer_get_controlled_element ();
		
		cd_keybinder_bind (myConfig.cShortcut, (CDBindkeyHandler) mixer_on_keybinding_pull, (gpointer)NULL);
		
		if (myDesklet)
		{
			gulong iOnEnterCallbackID = g_signal_handler_find (myDesklet->pWidget,
				G_SIGNAL_MATCH_FUNC,
				0,
				0,
				NULL,
				_cd_mixer_on_enter,
				NULL);
			if (myConfig.bHideScaleOnLeave && iOnEnterCallbackID <= 0)
			{
				g_signal_connect (G_OBJECT (myDesklet->pWidget),
					"enter-notify-event",
					G_CALLBACK (_cd_mixer_on_enter),
					NULL);
				g_signal_connect (G_OBJECT (myDesklet->pWidget),
					"leave-notify-event",
					G_CALLBACK (_cd_mixer_on_leave),
					NULL);
			}
			else if (! myConfig.bHideScaleOnLeave && iOnEnterCallbackID > 0)
			{
				g_signal_handler_disconnect (G_OBJECT (myDesklet->pWidget), iOnEnterCallbackID);
				gulong iOnLeaveCallbackID = g_signal_handler_find (myDesklet->pWidget,
					G_SIGNAL_MATCH_FUNC,
					0,
					0,
					NULL,
					_cd_mixer_on_leave,
					NULL);
				g_signal_handler_disconnect (G_OBJECT (myDesklet->pWidget), iOnLeaveCallbackID);
			}
		}
	}
	
	if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED && myDesklet)
	{
		cairo_dock_dialog_unreference (myData.pDialog);
		myData.pDialog = NULL;
		
		GtkWidget *box = gtk_hbox_new (FALSE, 0);
		myData.pScale = mixer_build_widget (FALSE);
		gtk_box_pack_end (GTK_BOX (box), myData.pScale, FALSE, FALSE, 0);
		gtk_widget_show_all (box);
		gtk_container_add (GTK_CONTAINER (myDesklet->pWidget), box);
	}
	
	//\_______________ On redessine notre icone.
	if (myData.pControledElement == NULL)
	{
		gchar *cImagePath = (myConfig.cBrokenIcon != NULL ? cairo_dock_generate_file_path (myConfig.cBrokenIcon) : g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/broken.svg", NULL));
		CD_APPLET_SET_IMAGE_ON_MY_ICON (cImagePath)
		g_free (cImagePath);
		//CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pBrokenSurface)
	}
	else
	{
		mixer_element_update_with_event (myData.pControledElement, 1);
		if (myData.iSidCheckVolume == 0)
			myData.iSidCheckVolume = g_timeout_add (1000, (GSourceFunc) mixer_check_events, (gpointer) NULL);
	}
CD_APPLET_RELOAD_END
