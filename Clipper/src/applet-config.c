/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-clipboard.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS

static void _cd_clipper_on_keybinding_pull (const char *keystring, gpointer user_data)
{
	if (myData.pItems == NULL)
	{
		gchar *cIconPath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_ICON_FILE);
		cairo_dock_show_temporary_dialog_with_icon (D_("No items yet."), myIcon, myContainer, 2000, cIconPath);
		g_free (cIconPath);
	}
	else
	{
		GtkWidget *pMenu = cd_clipper_build_items_menu ();
		cd_clipper_show_menu (pMenu, 0);
	}
}

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.iItemType = CD_CONFIG_GET_INTEGER ("Configuration", "item type");
	myConfig.bSeparateSelections = CD_CONFIG_GET_BOOLEAN ("Configuration", "separate");
	myConfig.iNbItems[CD_CLIPPER_CLIPBOARD] = CD_CONFIG_GET_INTEGER ("Configuration", "nb items");
	if (myConfig.bSeparateSelections)
	{
		if (myConfig.iItemType & CD_CLIPPER_PRIMARY)
			myConfig.iNbItems[CD_CLIPPER_PRIMARY] = CD_CONFIG_GET_INTEGER ("Configuration", "nb items2");
	}
	else
	{
		myConfig.iNbItems[CD_CLIPPER_BOTH] = myConfig.iNbItems[CD_CLIPPER_CLIPBOARD];
		myConfig.iNbItems[CD_CLIPPER_PRIMARY] = myConfig.iNbItems[CD_CLIPPER_CLIPBOARD];
	}
	myConfig.bPasteInClipboard = CD_CONFIG_GET_BOOLEAN ("Configuration", "paste clipboard");
	myConfig.bPasteInPrimary = CD_CONFIG_GET_BOOLEAN ("Configuration", "paste selection");
	myConfig.bEnableActions = CD_CONFIG_GET_BOOLEAN ("Configuration", "enable actions");
	myConfig.bMenuOnMouse = CD_CONFIG_GET_BOOLEAN ("Configuration", "menu on mouse");
	
	
	myConfig.bReplayAction = CD_CONFIG_GET_BOOLEAN ("Configuration", "replay action");
	myConfig.iActionMenuDuration = CD_CONFIG_GET_INTEGER ("Configuration", "action duration");
	
	myConfig.cShortCut = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	cd_keybinder_bind (myConfig.cShortCut, (CDBindkeyHandler)_cd_clipper_on_keybinding_pull, (gpointer)NULL);
	
	gsize length = 0;
	myConfig.pPersistentItems = CD_CONFIG_GET_STRING_LIST("Configuration", "persistent", &length);
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	if (myConfig.cShortCut)
	{
		cd_keybinder_unbind(myConfig.cShortCut, (CDBindkeyHandler)_cd_clipper_on_keybinding_pull);
		g_free (myConfig.cShortCut);
	}
	g_strfreev (myConfig.pPersistentItems);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	g_list_foreach (myData.pItems, (GFunc) cd_clipper_free_item, NULL);
	g_list_free (myData.pItems);
	
	g_list_foreach (myData.pActions, (GFunc)cd_clipper_free_action, NULL);
	g_list_free (myData.pActions);
	
	gtk_widget_destroy (myData.pActionMenu);
CD_APPLET_RESET_DATA_END
