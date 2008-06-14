/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-trashes-manager.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the dustbin applet for Cairo-Dock\n made by Fabrice Rey (fabounet@users.berlios.de)"))


CD_APPLET_ON_CLICK_BEGIN
	cd_message ("_Note_ : You can manage many Trash directories with this applet.\n Right click on its icon to see which Trash directories are already being monitored.");
	cd_dustbin_show_trash (NULL, "trash:/");
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Dustbin", pModuleSubMenu, CD_APPLET_MY_MENU)
	
	GString *sLabel = g_string_new ("");
	CdDustbin *pDustbin;
	GList *pElement;
	
	if (g_list_length (myData.pDustbinsList) == 1)
	{
		CD_APPLET_ADD_IN_MENU (D_("Show Trash"), cd_dustbin_show_trash, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (D_("Delete Trash"), cd_dustbin_delete_trash, CD_APPLET_MY_MENU)
	}
	else
	{
		CD_APPLET_ADD_SUB_MENU (D_("Show Trash"), pShowSubMenu, pModuleSubMenu)
		for (pElement = myData.pDustbinsList; pElement != NULL; pElement = pElement->next)
		{
			pDustbin = pElement->data;
			g_string_printf (sLabel, D_("Show %s"), pDustbin->cPath);
			CD_APPLET_ADD_IN_MENU_WITH_DATA (sLabel->str, cd_dustbin_show_trash, pShowSubMenu, pDustbin->cPath)
		}
		CD_APPLET_ADD_IN_MENU (D_("Show All"), cd_dustbin_show_trash, pShowSubMenu)
	
		CD_APPLET_ADD_SUB_MENU (D_("Delete Trash"), pDeleteSubMenu, pModuleSubMenu)
		for (pElement = myData.pDustbinsList; pElement != NULL; pElement = pElement->next)
		{
			pDustbin = pElement->data;
			g_string_printf (sLabel, D_("Delete %s"), pDustbin->cPath);
			CD_APPLET_ADD_IN_MENU_WITH_DATA (sLabel->str, cd_dustbin_delete_trash, pDeleteSubMenu, pDustbin->cPath)
		}
		CD_APPLET_ADD_IN_MENU (D_("Delete All"), cd_dustbin_delete_trash, pDeleteSubMenu)
	}
	
	g_string_free (sLabel, TRUE);
	
	CD_APPLET_ADD_ABOUT_IN_MENU (pModuleSubMenu)
CD_APPLET_ON_BUILD_MENU_END


static void _cd_dustbin_action_after_unmount (gboolean bMounting, gboolean bSuccess, const gchar *cName, Icon *icon, CairoContainer *pContainer)
{
	g_return_if_fail (myIcon != NULL && ! bMounting);
	gchar *cMessage;
	if (bSuccess)
	{
		cMessage = g_strdup_printf (_("%s is now unmounted"), cName);
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
			cairo_dock_fm_unmount_full (cURI, iVolumeID, _cd_dustbin_action_after_unmount, myIcon, myContainer);
		else
		{
			gchar * cDustbinPath = cairo_dock_fm_get_trash_path (CD_APPLET_RECEIVED_DATA, NULL);  // on laisse tomber les info pour l'instant ...
			g_return_val_if_fail (cDustbinPath != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
			cairo_dock_fm_move_file (cURI, cDustbinPath);
			if (! cd_dustbin_is_monitored (cDustbinPath))
			{
				cd_dustbin_add_one_dustbin (cDustbinPath, 0);
			}
			g_free (cDustbinPath);
			//cairo_dock_fm_delete_file (cURI);
		}
	}
	else
	{
		gchar *cHostname = NULL;
		GError *erreur = NULL;
		gchar *cFileName = g_filename_from_uri (CD_APPLET_RECEIVED_DATA, &cHostname, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : can't find valid URI for '%s' : %s", CD_APPLET_RECEIVED_DATA, erreur->message);
			g_error_free (erreur);
		}
		else if ((cHostname == NULL || strcmp (cHostname, "localhost") == 0) && myData.pDustbinsList != NULL)
		{
			CdDustbin *pDustbin = myData.pDustbinsList->data;
			gchar *cCommand = g_strdup_printf ("mv %s %s", cFileName, pDustbin->cPath);
			system (cCommand);
			g_free (cCommand);
		}
		g_free (cFileName);
		g_free (cHostname);
	}
	g_free (cName);
	g_free (cURI);
	g_free (cIconName);
CD_APPLET_ON_DROP_DATA_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	GString *sInfo = g_string_new ("");
	g_string_printf (sInfo, "%.2fMb for %d files in all dustbins :", 1.*myData.iSize/(1024*1024), myData.iNbFiles);
	CdDustbin *pDustbin;
	GList *pElement;
	for (pElement = myData.pDustbinsList; pElement != NULL; pElement = pElement->next)
	{
		pDustbin = pElement->data;
		g_string_append_printf (sInfo, "\n  %.2fM for %d files for in %s", 1.*pDustbin->iSize/(1024*1024), pDustbin->iNbFiles, pDustbin->cPath);
	}
	
	cairo_dock_show_temporary_dialog_with_icon (sInfo->str, myIcon, myContainer, 5000, myData.cDialogIconPath);
	
	g_string_free (sInfo, TRUE);
CD_APPLET_ON_MIDDLE_CLICK_END
