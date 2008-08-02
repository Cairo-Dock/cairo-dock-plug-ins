/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-widget.h"
#include "applet-init.h"

CD_APPLET_DEFINITION ("weblets", 1, 6, 2, CAIRO_DOCK_CATEGORY_ACCESSORY)

//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN

	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT

	if (myDesklet != NULL)  // on cree le weblet pour avoir qqch a afficher dans le desklet.
	{
		weblet_build_and_show (myApplet);

		// mise en place du timer
		myData.pRefreshTimer = cairo_dock_new_measure_timer (myConfig.iReloadTimeout,
			NULL,
			NULL,
			cd_weblets_refresh_page,
			myApplet);
		cairo_dock_launch_measure (myData.pRefreshTimer); // ceci lance au moins une fois le chargement de la page
		if( myConfig.iReloadTimeout == 0 )
		{
			// oublions le, il ne sert deja plus a rien...
			myData.pRefreshTimer = NULL;
		}
	}

CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT



CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (! myData.pGtkMozEmbed)
		{
			if (myDesklet != NULL)  // on cree le terminal pour avoir qqch a afficher dans le desklet.
				weblet_build_and_show (myApplet);
		}
		else if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
		{
			if (myDesklet != NULL)  // il faut passer du dialogue au desklet.
			{
				myData.pGtkMozEmbed = cairo_dock_steal_widget_from_its_container (myData.pGtkMozEmbed);
				cairo_dock_dialog_unreference (myData.dialog);
				myData.dialog = NULL;
				cairo_dock_add_interactive_widget_to_desklet (myData.pGtkMozEmbed, myDesklet);
				//myDesklet->renderer = term_draw_in_desklet;
				cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
			}
			else  // il faut passer du desklet au dialogue
			{
				myData.dialog = cairo_dock_build_dialog (D_("Terminal"), myIcon, myContainer, NULL, myData.pGtkMozEmbed, GTK_BUTTONS_NONE, NULL, NULL, NULL);
				cairo_dock_hide_dialog (myData.dialog);
			}
		}

		// on remet en place un timer tout frais
		if( myData.pRefreshTimer )
		{
			cairo_dock_free_measure_timer( myData.pRefreshTimer );
			myData.pRefreshTimer = NULL;
		}
		myData.pRefreshTimer = cairo_dock_new_measure_timer (myConfig.iReloadTimeout,
			NULL,
			NULL,
			cd_weblets_refresh_page,
			myApplet);
		cairo_dock_launch_measure (myData.pRefreshTimer); // ceci lance au moins une fois le chargement de la page
		if( myConfig.iReloadTimeout == 0 )
		{
			myData.pRefreshTimer = NULL;
		}
	}
CD_APPLET_RELOAD_END
