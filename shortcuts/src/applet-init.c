/*********************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
#include "stdlib.h"
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-bookmarks.h"
#include "applet-load-icons.h"
#include "applet-draw.h"
#include "applet-struct.h"
#include "applet-init.h"

AppletConfig myConfig;
AppletData myData;


CD_APPLET_DEFINITION ("shortcuts", 1, 5, 0)


CD_APPLET_INIT_BEGIN (erreur)
	if (myIcon->acName == NULL || *myIcon->acName == '\0')
		myIcon->acName = g_strdup (SHORTCUTS_DEFAULT_NAME);
	
	if (myDesklet != NULL)
	{
		myDesklet->renderer = cd_shortcuts_draw_in_desklet;
	}
	
	//\_______________ On charge les icones dans un sous-dock.
	cd_shortcuts_launch_measure ();  // asynchrone
	
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
	
	reset_data ();
	reset_config ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (myDesklet != NULL)
	{
		myDesklet->renderer = cd_shortcuts_draw_in_desklet;
	}
	
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On charge les icones dans un sous-dock.
		reset_data ();
		
		if (myIcon->acName == NULL || *myIcon->acName == '\0')
			myIcon->acName = g_strdup (SHORTCUTS_DEFAULT_NAME);
		
		cd_shortcuts_launch_measure ();  // asynchrone
	}
	else if (myDesklet != NULL)
	{
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
		cd_shortcuts_load_tree (myData.pDeskletIconList, pCairoContext);
		
		GList* ic;
		Icon *icon;
		for (ic = myData.pDeskletIconList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			icon->fWidth = 48 * MIN (myData.fTreeWidthFactor, myData.fTreeHeightFactor);
			icon->fHeight = 48 * MIN (myData.fTreeWidthFactor, myData.fTreeHeightFactor);
			cairo_dock_fill_icon_buffers (icon, pCairoContext, 1, CAIRO_DOCK_HORIZONTAL, FALSE);
		}
		cairo_destroy (pCairoContext);
	}
	else
	{
		// rien a faire, cairo-dock va recharger notre sous-dock.
	}
CD_APPLET_RELOAD_END
