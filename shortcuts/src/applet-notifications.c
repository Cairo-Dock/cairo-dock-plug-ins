/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-bookmarks.h"
#include "applet-struct.h"
#include "applet-notifications.h"


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (CD_APPLET_CLICKED_CONTAINER == myDock)  // clic sur l'icone principale dans le dock CD_APPLET_CLICKED_CONTAINER n'est jamais NULL).
	{
		gboolean bDesktopIsVisible = cairo_dock_desktop_is_visible ();
		g_print ("bDesktopIsVisible : %d\n", bDesktopIsVisible);
		cairo_dock_show_hide_desktop (! bDesktopIsVisible);
	}
	else if (CD_APPLET_CLICKED_ICON != NULL && (CD_APPLET_CLICKED_ICON->iType == 6 || CD_APPLET_CLICKED_ICON->iVolumeID != 0))  // clic sur une icone du sous-dock ou du desklet, et de type 'point de montage'.
	{
		gboolean bIsMounted = FALSE;
		gchar *cActivationURI = cairo_dock_fm_is_mounted (CD_APPLET_CLICKED_ICON->cBaseURI, &bIsMounted);
		cd_message ("  cActivationURI : %s; bIsMounted : %d\n", cActivationURI, bIsMounted);
		g_free (cActivationURI);

		if (! bIsMounted)
		{
			g_print ("ON MONTE\n");
			cairo_dock_fm_mount (CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER);
		}
		else
		{
			g_print ("ON DEMONTE\n");
			cairo_dock_fm_unmount (CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER);
		}
	}
CD_APPLET_ON_MIDDLE_CLICK_END


static void _cd_shortcuts_remove_bookmark (GtkMenuItem *menu_item, gchar *cURI)
{
	cd_shortcuts_remove_one_bookmark (cURI);
}
static void _cd_shortcuts_show_disk_info (GtkMenuItem *menu_item, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	Icon *pIcon = data[1];
	CairoContainer *pContainer = data[2];
	g_print ("%s (%s)\n", __func__, pIcon->acCommand);
	
	gchar *cText = g_strdup_printf ("pouet");
	
	cairo_dock_show_temporary_dialog_with_icon (cText, pIcon, pContainer, 7000, "same icon");
	g_free (cText);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	static gpointer *data = NULL;
	if (CD_APPLET_CLICKED_CONTAINER == myDock)  // clic sur l'icone principale dans le dock CD_APPLET_CLICKED_CONTAINER n'est jamais NULL).
	{
		GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
	}
	if (CD_APPLET_CLICKED_ICON != NULL && CD_APPLET_CLICKED_ICON->iType == 10)
	{
		//cd_message (" menu sur %s(%s)", CD_APPLET_CLICKED_ICON->acName, CD_APPLET_CLICKED_ICON->cBaseURI);
		CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Remove this bookmark"), _cd_shortcuts_remove_bookmark, CD_APPLET_MY_MENU, CD_APPLET_CLICKED_ICON->cBaseURI);
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	if (CD_APPLET_CLICKED_ICON != NULL && CD_APPLET_CLICKED_ICON->iType == 6 && CD_APPLET_CLICKED_ICON->acCommand != NULL)
	{
		if (data == NULL)
			data = g_new (gpointer, 3);
		data[0] = myApplet;
		data[1] = CD_APPLET_CLICKED_ICON;
		data[2] = CD_APPLET_CLICKED_CONTAINER;
		CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("Get disk info"), _cd_shortcuts_show_disk_info, CD_APPLET_MY_MENU, data);
	}
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_DROP_DATA_BEGIN
	if (myDock && myIcon->pSubDock == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	cd_message ("  nouveau signet : %s", CD_APPLET_RECEIVED_DATA);
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
