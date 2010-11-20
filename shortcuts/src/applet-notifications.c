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
#include <glib/gi18n.h>

#include "applet-bookmarks.h"
#include "applet-struct.h"
#include "applet-disk-usage.h"
#include "applet-notifications.h"

static void _on_volume_mounted (gboolean bMounting, gboolean bSuccess, const gchar *cName, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL);
	
	Icon *pIcon = cairo_dock_get_icon_with_base_uri (CD_APPLET_MY_ICONS_LIST, cURI);
	CD_APPLET_LEAVE_IF_FAIL (pIcon != NULL);
	
	//g_print ("%s (%s , %d)\n", __func__, cURI, bSuccess);
	if (! bSuccess)  // en cas de montage reussi, on aura un dialogue via les evenements.
	{
		cairo_dock_remove_dialog_if_any (pIcon);
		cairo_dock_show_temporary_dialog_with_icon_printf (
			bMounting ? _("failed to mount %s") : _("Failed to unmount %s"),
			pIcon, pContainer,
			4000,
			"same icon",  // petit risque de n'avoir pas encore d'image a afficher, pas bien grave.
			pIcon->cName);
	}
	CD_APPLET_LEAVE ();
}

static void _open_on_mount (gboolean bMounting, gboolean bSuccess, const gchar *cName, const gchar *cURI, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	CD_APPLET_LEAVE_IF_FAIL (pContainer != NULL);
	
	//g_print ("%s (%s)\n", __func__, cURI);
	Icon *pIcon = cairo_dock_get_icon_with_base_uri (CD_APPLET_MY_ICONS_LIST, cURI);
	CD_APPLET_LEAVE_IF_FAIL (pIcon != NULL);
	
	if (bSuccess)  // montage reussi.
	{
		cairo_dock_fm_launch_uri (pIcon->cCommand);
	}
	else
	{
		cairo_dock_remove_dialog_if_any (pIcon);
		cairo_dock_show_temporary_dialog_with_icon_printf (
			bMounting ? _("failed to mount %s") : _("Failed to unmount %s"),
			pIcon, pContainer,
			4000,
			"same icon",  // petit risque de n'avoir pas encore d'image a afficher, pas bien grave.
			pIcon->cName);
	}
	CD_APPLET_LEAVE ();
}

CD_APPLET_ON_CLICK_BEGIN
	if (CD_APPLET_CLICKED_ICON == myIcon)  // clic sur l'icone principale -> on affiche un message en cas de probleme, sinon on laisse passer la notification.
	{
		if (CD_APPLET_MY_ICONS_LIST == NULL)
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			if (g_iDesktopEnv == CAIRO_DOCK_KDE)
				cairo_dock_show_temporary_dialog_with_icon (D_("Sorry, this applet is not yet available for KDE."), myIcon, myContainer, 6000., "same icon");
			else
				cairo_dock_show_temporary_dialog_with_icon (D_("No disks or bookmarks were found."), myIcon, myContainer, 6000., "same icon");
		}
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);  // on laisse passer la notification (pour ouvrir le sous-dock au clic).
	}
	else if (CD_APPLET_CLICKED_ICON != NULL)  // clic sur une des icones de la liste.
	{
		if (CD_APPLET_CLICKED_ICON->iType == CD_DRIVE_GROUP && CD_APPLET_CLICKED_ICON->iVolumeID)  // clic sur un point de montage.
		{
			gboolean bIsMounted = FALSE;
			gchar *cActivationURI = cairo_dock_fm_is_mounted (CD_APPLET_CLICKED_ICON->cBaseURI, &bIsMounted);
			g_free (cActivationURI);
			if (bIsMounted)
			{
				cairo_dock_fm_launch_uri (CD_APPLET_CLICKED_ICON->cCommand);
			}
			else
			{
				cairo_dock_fm_mount_full (CD_APPLET_CLICKED_ICON->cBaseURI, CD_APPLET_CLICKED_ICON->iVolumeID, (CairoDockFMMountCallback) _open_on_mount, myApplet);
			}
		}
		else if (CD_APPLET_CLICKED_ICON->iType == CD_BOOKMARK_GROUP)  // clic sur un signet, il peut etre place sur un volume non monte.
		{
			GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
			CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
			
			/// chercher un volume contenant le signet
			
			/// si trouve et non monte, on le monte
			
			/// si non trouve, on cree l'URI et on monte.
			
			/*gchar *cVolumeUri = g_strdup (CD_APPLET_CLICKED_ICON->cCommand);
			gchar *str = strchr (cVolumeUri, ':');
			if (str && strlen (str) > 3)
			{
				str = strchr (str, '/');
			}
			g_print ("bookmark '%s' on '%s'\n", CD_APPLET_CLICKED_ICON->cCommand, cVolumeUri);
			gboolean bIsMounted = FALSE;
			gchar *cActivationURI = cairo_dock_fm_is_mounted (CD_APPLET_CLICKED_ICON->cBaseURI, &bIsMounted);
			g_free (cActivationURI);
			* 
			if (!bIsMounted)
			{
				g_print (" volume not mounted\n");
				cairo_dock_fm_mount_full (cVolumeUri, 1, (CairoDockFMMountCallback) _open_on_mount, myApplet);
			}
			else
			{
				cairo_dock_fm_launch_uri (CD_APPLET_CLICKED_ICON->cCommand);
			}
			g_free (cVolumeUri);*/
			cairo_dock_fm_launch_uri (CD_APPLET_CLICKED_ICON->cCommand);
		}
		else
		{
			cairo_dock_fm_launch_uri (CD_APPLET_CLICKED_ICON->cCommand);
		}
	}
CD_APPLET_ON_CLICK_END



static void _mount_unmount (Icon *pIcon, CairoContainer *pContainer, CairoDockModuleInstance *myApplet)
{
	gboolean bIsMounted = FALSE;
	gchar *cActivationURI = cairo_dock_fm_is_mounted (pIcon->cBaseURI, &bIsMounted);
	g_free (cActivationURI);

	if (! bIsMounted)
	{
		cairo_dock_fm_mount_full (pIcon->cBaseURI, pIcon->iVolumeID, (CairoDockFMMountCallback) _on_volume_mounted, myApplet);
	}
	else
	{
		cairo_dock_fm_unmount_full (pIcon->cBaseURI, pIcon->iVolumeID, (CairoDockFMMountCallback) _on_volume_mounted, myApplet);
		
		cairo_dock_show_temporary_dialog_with_icon (D_("Unmouting this volume ..."), pIcon, pContainer, 15000., "same icon");  // le dialogue sera enleve lorsque le volume sera demonte.
	}
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (CD_APPLET_CLICKED_ICON == myIcon)  // clic sur l'icone principale.
	{
		cairo_dock_fm_launch_uri (g_getenv ("HOME"));
	}
	else if (CD_APPLET_CLICKED_ICON != NULL && (CD_APPLET_CLICKED_ICON->iType == CD_DRIVE_GROUP || CD_APPLET_CLICKED_ICON->iVolumeID > 0))  // clic sur une icone du sous-dock ou du desklet, et de type 'point de montage'.
	{
		_mount_unmount (CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER, myApplet);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


static void _cd_shortcuts_remove_bookmark (GtkMenuItem *menu_item, const gchar *cURI)
{
	cd_shortcuts_remove_one_bookmark (cURI);
}
static void _cd_shortcuts_rename_bookmark (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	Icon *pIcon = data[1];
	CairoContainer *pContainer = data[2];
	CD_APPLET_ENTER;
	gchar *cNewName = cairo_dock_show_demand_and_wait (D_("Enter a name for this bookmark:"),
		pIcon, pContainer,
		pIcon->cName);
	if (cNewName != NULL)
	{
		cd_shortcuts_rename_one_bookmark (pIcon->cCommand, cNewName);
		g_free (cNewName);
	}
	CD_APPLET_LEAVE ();
}
static void _cd_shortcuts_eject (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	CD_APPLET_ENTER;
	Icon *pIcon = data[1];
	CairoContainer *pContainer = data[2];
	
	cairo_dock_fm_eject_drive (pIcon->cBaseURI);
	CD_APPLET_LEAVE ();
}
static void _cd_shortcuts_unmount (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	CD_APPLET_ENTER;
	Icon *pIcon = data[1];
	CairoContainer *pContainer = data[2];
	
	_mount_unmount (pIcon, pContainer, myApplet);
	CD_APPLET_LEAVE ();
}
static void _cd_shortcuts_show_disk_info (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	CD_APPLET_ENTER;
	Icon *pIcon = data[1];
	CairoContainer *pContainer = data[2];
	
	gchar *cInfo = cd_shortcuts_get_disk_info (pIcon->cCommand, pIcon->cName);
	cairo_dock_show_temporary_dialog_with_icon (cInfo, pIcon, pContainer, 15000, "same icon");
	g_free (cInfo);
	CD_APPLET_LEAVE ();
}
static void _open_home_dir (GtkMenuItem *menu_item, gpointer data)
{
	cairo_dock_fm_launch_uri (g_getenv ("HOME"));
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	static gpointer *data = NULL;
	if (data == NULL)
		data = g_new (gpointer, 3);
	data[0] = myApplet;
	data[1] = CD_APPLET_CLICKED_ICON;
	data[2] = CD_APPLET_CLICKED_CONTAINER;
	
	if (CD_APPLET_CLICKED_ICON == myIcon)  // clic sur l'icone principale (mode dock donc).
	{
		GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Open Home directory"), D_("middle-click"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_OPEN, _open_home_dir, CD_APPLET_MY_MENU);
		g_free (cLabel);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
	}
	else if (CD_APPLET_CLICKED_ICON != NULL)  // clic sur un item.
	{
		if (CD_APPLET_CLICKED_ICON->iType == CD_BOOKMARK_GROUP)  // clic sur un signet.
		{
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Rename this bookmark"),  GTK_STOCK_SAVE_AS, _cd_shortcuts_rename_bookmark, CD_APPLET_MY_MENU, data);
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Remove this bookmark"), GTK_STOCK_REMOVE, _cd_shortcuts_remove_bookmark, CD_APPLET_MY_MENU, CD_APPLET_CLICKED_ICON->cBaseURI);
			CD_APPLET_LEAVE (CAIRO_DOCK_INTERCEPT_NOTIFICATION);
		}
		else if (CD_APPLET_CLICKED_ICON->iType == CD_DRIVE_GROUP && CD_APPLET_CLICKED_ICON->cBaseURI != NULL)  // clic sur un volume.
		{
			if (cairo_dock_fm_can_eject (CD_APPLET_CLICKED_ICON->cBaseURI))
				CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Eject"), GTK_STOCK_DISCONNECT, _cd_shortcuts_eject, CD_APPLET_MY_MENU, data);
			
			gboolean bIsMounted = FALSE;
			gchar *cURI = cairo_dock_fm_is_mounted (CD_APPLET_CLICKED_ICON->cBaseURI, &bIsMounted);
			g_free (cURI);
			gchar *cLabel = g_strdup_printf ("%s (%s)", bIsMounted ? D_("Unmount") : D_("Mount"), D_("middle-click"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (cLabel, GTK_STOCK_DISCONNECT, _cd_shortcuts_unmount, CD_APPLET_MY_MENU, data);
			g_free (cLabel);
			
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Get disk info"), GTK_STOCK_PROPERTIES, _cd_shortcuts_show_disk_info, CD_APPLET_MY_MENU, data);
		}
	}
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_DROP_DATA_BEGIN
	if (myDock && myIcon->pSubDock == NULL)
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
	cd_message ("  new bookmark : %s", CD_APPLET_RECEIVED_DATA);
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
		if (! iVolumeID && ! bIsDirectory)
		{
			cd_warning ("this can't be a bookmark");
			cairo_dock_show_temporary_dialog_with_icon (D_("Only folders can be bookmarked."), myIcon, myContainer, 4000, "same icon");
		}
		else
		{
			cd_shortcuts_add_one_bookmark (cURI);
		}
	}
	else
	{
		cd_warning ("couldn't get info about '%s', we won't add it", CD_APPLET_RECEIVED_DATA);
	}
	g_free (cName);
	g_free (cURI);
	g_free (cIconName);
CD_APPLET_ON_DROP_DATA_END


gboolean cd_shortcuts_free_data (CairoDockModuleInstance *myApplet, Icon *pIcon)
{
	CDDiskUsage *pDiskUsage = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pDiskUsage == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	g_free (pDiskUsage);
	
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
