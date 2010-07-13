/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-trashes-manager.h"
#include "applet-notifications.h"

static void _cd_dustbin_delete_trash (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cairo_dock_fm_empty_trash ();
}

static void _cd_dustbin_show_trash (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cairo_dock_fm_launch_uri ("trash:/"/**myData.cDustbinPath*/);  // on force l'utilisation de trash:/ ici, car on sait que tous les backends sauront l'ouvrir.
}

static void _cd_dustbin_show_info (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	/**GString *sInfo = g_string_new ("");
	if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
		g_string_printf (sInfo, "%.2fMb for %d files in all dustbins :", 1.*myData.iSize/(1024*1024), myData.iNbFiles);
	else
		g_string_printf (sInfo, "%d elements in all dustbins :", myData.iNbTrashes);
	
	CdDustbin *pDustbin;
	GList *pElement;
	for (pElement = myData.pDustbinsList; pElement != NULL; pElement = pElement->next)
	{
		pDustbin = pElement->data;
		if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_NB_FILES || myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
			g_string_append_printf (sInfo, "\n  %.2fM for %d files in %s", 1.*pDustbin->iSize/(1024*1024), pDustbin->iNbFiles, pDustbin->cPath);
		else
			g_string_append_printf (sInfo, "\n  %d elements in %s", pDustbin->iNbTrashes, pDustbin->cPath);
	}
	
	cairo_dock_remove_dialog_if_any (myIcon);
	cairo_dock_show_temporary_dialog_with_icon (sInfo->str, myIcon, myContainer, 5000, myData.cDialogIconPath);
	
	g_string_free (sInfo, TRUE);*/
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pModuleSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	
	CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Show Trash (click)"), _cd_dustbin_show_trash, CD_APPLET_MY_MENU, NULL);
	CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Empty Trash (middle-click)"), _cd_dustbin_delete_trash, CD_APPLET_MY_MENU, NULL);
	
	CD_APPLET_ADD_IN_MENU (D_("Display dustbins information"), _cd_dustbin_show_info, CD_APPLET_MY_MENU);
	
	CD_APPLET_ADD_ABOUT_IN_MENU (pModuleSubMenu);
CD_APPLET_ON_BUILD_MENU_END


static void _cd_dustbin_action_after_unmount (gboolean bMounting, gboolean bSuccess, const gchar *cName, gpointer data)
{
	g_return_if_fail (myIcon != NULL && ! bMounting);
	gchar *cMessage;
	if (bSuccess)
	{
		cMessage = g_strdup_printf (_("%s successfully unmounted"), cName);
	}
	else
	{
		cMessage = g_strdup_printf (_("failed to unmount %s"), cName);
		
	}
	cairo_dock_show_temporary_dialog (cMessage, myIcon, myContainer, 4000);
	g_free (cMessage);
}
CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message ("  %s --> a la poubelle !", CD_APPLET_RECEIVED_DATA);
	gchar *cName=NULL, *cURI=NULL, *cIconName=NULL;
	gboolean bIsDirectory;
	int iVolumeID = 0;
	double fOrder;
	if (cairo_dock_fm_get_file_info (CD_APPLET_RECEIVED_DATA,
		&cName,
		&cURI,
		&cIconName,
		&bIsDirectory,
		&iVolumeID,
		&fOrder,
		0))
	{
		if (iVolumeID > 0)
			cairo_dock_fm_unmount_full (cURI, iVolumeID, (CairoDockFMMountCallback) _cd_dustbin_action_after_unmount, myApplet);
		else
			cairo_dock_fm_delete_file (cURI, FALSE);
	}
	else
	{
		cd_warning ("can't get info for '%s'", CD_APPLET_RECEIVED_DATA);
	}
	g_free (cName);
	g_free (cURI);
	g_free (cIconName);
CD_APPLET_ON_DROP_DATA_END


CD_APPLET_ON_CLICK_BEGIN
	_cd_dustbin_show_trash (NULL, myApplet);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cairo_dock_fm_empty_trash ();
CD_APPLET_ON_MIDDLE_CLICK_END
