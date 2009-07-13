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
#include "applet-struct.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("shortcuts",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_DESKTOP,
	N_("An applet that let you access quickly to all of your shortcuts.\n"
	"It can manage disks, network points, and Nautilus bookmarks (even if you don't have Nautilus).\n"
	"Drag and drop a folder on the main icon or the sub-dock to add a bookmark.\n"
	"Middle-click on the main icon to acces your desktop easily.\n"
	"Middle-click on a mounting point icon to (un)mount is quickly.\n"
	"The applet can also display valuable information about your disks, like free space, type, etc."),
	"Fabounet (Fabrice Rey) & Jackass (Benjamin SANS)")


CD_APPLET_INIT_BEGIN
	CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
	
	//\_______________ On charge les icones dans un sous-dock.
	myData.pTask = cairo_dock_new_task (0,
		(CairoDockGetDataAsyncFunc) cd_shortcuts_get_shortcuts_data,
		(CairoDockUpdateSyncFunc) cd_shortcuts_build_shortcuts_from_data,
		myApplet);
	cairo_dock_launch_task (myData.pTask);
	
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		//\_______________ On charge les icones dans un sous-dock.
		cd_shortcuts_reset_all_datas (myApplet);  // stoppe les mesures.
		
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;  // set the default icon if none is specified in conf.
		
		myData.pTask = cairo_dock_new_task (0,
			(CairoDockGetDataAsyncFunc) cd_shortcuts_get_shortcuts_data,
			(CairoDockUpdateSyncFunc) cd_shortcuts_build_shortcuts_from_data,
			myApplet);
		cairo_dock_launch_task (myData.pTask);
	}
	else if (myDesklet)  // on recharge juste la vue du desklet qui a change de taille.
	{
		const gchar *cDeskletRendererName = NULL;
		switch (myConfig.iDeskletRendererType)
		{
			case CD_DESKLET_SLIDE :
			default :
				cDeskletRendererName = "Slide";
			break ;
			
			case CD_DESKLET_TREE :
				cDeskletRendererName = "Tree";
			break ;
		}
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA (cDeskletRendererName, NULL);
	}
	else
	{
		// rien a faire, cairo-dock va recharger notre sous-dock.
	}
CD_APPLET_RELOAD_END

