/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

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

AppletConfig myConfig;
AppletData myData;


CD_APPLET_DEFINITION ("Xgamma", 1, 4, 7)


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
			return NULL;
		}
		
		int MajorVersion, MinorVersion;
		if (!XF86VidModeQueryVersion(dpy, &MajorVersion, &MinorVersion))
		{
			g_set_error (erreur, 1, 1, "%s () : unable to query video extension version", __func__);
			return NULL;
		}
		
		int EventBase, ErrorBase;
		if (!XF86VidModeQueryExtension(dpy, &EventBase, &ErrorBase))
		{
			g_set_error (erreur, 1, 1, "%s () : unable to query video extension information", __func__);
			return NULL;
		}
		
		myData.bVideoExtensionOK = TRUE;
	}
	
	
	
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	
	//\_________________ On libere toutes nos ressources.
	reset_config ();
	reset_data ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		
	}
	else
	{
		
	}
CD_APPLET_RELOAD_END
