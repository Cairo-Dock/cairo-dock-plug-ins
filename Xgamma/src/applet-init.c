/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#include <ctype.h>
#include <stdlib.h>

#include <cairo-dock.h>

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-xgamma.h"
#include "applet-struct.h"
#include "applet-init.h"

static gboolean s_bVideoExtensionChecked = FALSE;


CD_APPLET_DEFINITION ("Xgamma", 1, 5, 4, CAIRO_DOCK_CATEGORY_DESKTOP)


CD_APPLET_INIT_BEGIN (erreur)
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	
	if (! s_bVideoExtensionChecked)
	{
		s_bVideoExtensionChecked = TRUE;
		
		const Display *dpy = cairo_dock_get_Xdisplay ();
		if (dpy == NULL)
		{
			g_set_error (erreur, 1, 1, "%s () : unable to get X display", __func__);
			return ;
		}
		
		int MajorVersion, MinorVersion;
		if (!XF86VidModeQueryVersion(dpy, &MajorVersion, &MinorVersion))
		{
			g_set_error (erreur, 1, 1, "%s () : unable to query video extension version", __func__);
			return ;
		}
		
		int EventBase, ErrorBase;
		if (!XF86VidModeQueryExtension(dpy, &EventBase, &ErrorBase))
		{
			g_set_error (erreur, 1, 1, "%s () : unable to query video extension information", __func__);
			return ;
		}
		
		myData.bVideoExtensionOK = TRUE;
	}
	
	if (myDesklet != NULL)  // on cree le widget pour avoir qqch a afficher dans le desklet.
		xgamma_build_and_show_widget ();
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (! myData.pWidget)
		{
			if (myDesklet != NULL)  // on cree le widget pour avoir qqch a afficher dans le desklet.
				xgamma_build_and_show_widget ();
		}
		else if (CD_APPLET_MY_CONTAINER_TYPE_CHANGED)
		{
			if (myDesklet != NULL)  // il faut passer du dialogue au desklet.
			{
				myData.pWidget = cairo_dock_steal_widget_from_its_container (myData.pWidget);
				cairo_dock_dialog_unreference (myData.pDialog);
				myData.pDialog = NULL;
				cairo_dock_add_interactive_widget_to_desklet (myData.pWidget, myDesklet);
				//myDesklet->renderer = xgamma_draw_in_desklet;
				cairo_dock_set_desklet_renderer_by_name (myDesklet, NULL, NULL, ! CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
			}
			else  // il faut passer du desklet au dialogue
			{
				myData.pDialog = cairo_dock_build_dialog (_D("Set up gamma :"),
					myIcon,
					myDock,
					NULL,
					myData.pWidget,
					GTK_BUTTONS_OK_CANCEL,
					(CairoDockActionOnAnswerFunc) xgamma_apply_values,
					NULL,
					NULL);
				cairo_dock_hide_dialog (myData.pDialog);
			}
		}
	}
CD_APPLET_RELOAD_END
