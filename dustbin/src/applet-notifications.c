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
	int iAnswer = GTK_RESPONSE_YES;
	if (myConfig.bAskBeforeDelete)
	{
		iAnswer = cairo_dock_ask_question_and_wait (D_("You're about to delete all files in all dustbins. Sure ?"), myIcon, myContainer);
	}
	
	if (iAnswer == GTK_RESPONSE_YES)
		cairo_dock_fm_empty_trash ();
}

static void _cd_dustbin_show_trash (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cairo_dock_fm_launch_uri ("trash:/"/**myData.cDustbinPath*/);  // on force l'utilisation de trash:/ ici, car on sait que tous les backends sauront l'ouvrir.
}

static void _cd_dustbin_show_info (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	gsize iSize, iNbFiles;
	gint iCancel = 0;
	if (myConfig.iQuickInfoType == CD_DUSTBIN_INFO_WEIGHT)
	{
		iSize = myData.iMeasure;
	}
	else
	{
		iSize = cairo_dock_fm_measure_diretory (myData.cDustbinPath, 0, TRUE, &iCancel);
	}
	iNbFiles = cairo_dock_fm_measure_diretory (myData.cDustbinPath, 0, FALSE, &iCancel);
	
	cairo_dock_remove_dialog_if_any (myIcon);
	cairo_dock_show_temporary_dialog_with_icon_printf (D_("%s :\n %d files\n %.2f %s"),
		myIcon, myContainer,
		5000,
		"same icon",
		D_("The trash contains"),
		iNbFiles,
		(iSize > 1e6 ? (iSize >> 10) / 1024. : iSize / 1024.),
		(iSize > 1e6 ? D_("Mo") : D_("Ko")));
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
	cd_message ("  '%s' --> a la poubelle !", CD_APPLET_RECEIVED_DATA);
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
	_cd_dustbin_delete_trash (NULL, myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END
