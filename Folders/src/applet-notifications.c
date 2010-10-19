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
#define __USE_POSIX 1
#include <time.h>

#include "applet-struct.h"
#include "applet-notifications.h"


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (CD_APPLET_CLICKED_ICON == myIcon)
	{
		if (CD_APPLET_MY_ICONS_LIST == NULL)  // repertoire vide, ou illisible, ou non defini.
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			if (myConfig.cDirPath == NULL)
				cairo_dock_show_temporary_dialog_with_icon (D_("Open the configuration of the applet to choose a folder to import."),
					myIcon, myContainer,
					8000.,
					myConfig.iSubdockViewType == 0 ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
			else
				cairo_dock_show_temporary_dialog_with_icon_printf ("%s :\n%s",
					myIcon, myContainer,
					4000.,
					myConfig.iSubdockViewType == 0 ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
					myConfig.cDirPath,
					D_("Empty or unreadable folder."));
		}
		else
			CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);  // on laisse passer la notification (pour ouvrir le sous-dock au clic).
	}
	else if (CD_APPLET_CLICKED_ICON != NULL)
	{
		//g_print ("clic on %s\n", CD_APPLET_CLICKED_ICON->cBaseURI);
		if (CD_APPLET_CLICKED_ICON->iVolumeID == -1)
		{
			//g_print ("folder\n");
			
			/// lister le repertoire dans un thread.
			
			/// lancer une animation de transition.
			
			
			/// en attendant, on ouvre le repertoire...
			cairo_dock_fm_launch_uri (CD_APPLET_CLICKED_ICON->cBaseURI);
		}
		else
		{
			cairo_dock_fm_launch_uri (CD_APPLET_CLICKED_ICON->cBaseURI);
		}
	}
CD_APPLET_ON_CLICK_END


//\___________ Same as ON_CLICK, but with middle-click.
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (CD_APPLET_CLICKED_ICON == myIcon)
	{
		cairo_dock_fm_launch_uri (myConfig.cDirPath);
	}
	else if (CD_APPLET_CLICKED_ICON != NULL && CD_APPLET_CLICKED_ICON->iVolumeID != 0) // clic sur un des repertoires.
	{
		cairo_dock_fm_launch_uri (CD_APPLET_CLICKED_ICON->cBaseURI);
	}
CD_APPLET_ON_MIDDLE_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.

static void _cd_folders_show_file_properties (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoDock *pDock = data[1];
	//g_print ("%s (%s)\n", __func__, icon->cName);

	guint64 iSize = 0;
	time_t iLastModificationTime = 0;
	gchar *cMimeType = NULL;
	int iUID=0, iGID=0, iPermissionsMask=0;
	if (cairo_dock_fm_get_file_properties (icon->cBaseURI, &iSize, &iLastModificationTime, &cMimeType, &iUID, &iGID, &iPermissionsMask))
	{
		GtkWidget *pDialog = gtk_message_dialog_new (GTK_WINDOW (pDock->container.pWidget),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			"Properties :");

		GString *sInfo = g_string_new ("");
		g_string_printf (sInfo, "<b>%s</b>", icon->cName);

		GtkWidget *pLabel= gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);

		GtkWidget *pFrame = gtk_frame_new (NULL);
		gtk_container_set_border_width (GTK_CONTAINER (pFrame), 3);
		gtk_frame_set_label_widget (GTK_FRAME (pFrame), pLabel);
		gtk_frame_set_shadow_type (GTK_FRAME (pFrame), GTK_SHADOW_OUT);
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG (pDialog)->vbox), pFrame);

		GtkWidget *pVBox = gtk_vbox_new (FALSE, 3);
		gtk_container_add (GTK_CONTAINER (pFrame), pVBox);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		g_string_printf (sInfo, "<u>Size</u> : %lld bytes", iSize);
		if (iSize > 1024*1024)
			g_string_append_printf (sInfo, " (%.1f Mo)", 1. * iSize / 1024 / 1024);
		else if (iSize > 1024)
			g_string_append_printf (sInfo, " (%.1f Ko)", 1. * iSize / 1024);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		struct tm epoch_tm;
		localtime_r (&iLastModificationTime, &epoch_tm);  // et non pas gmtime_r.
		gchar *cTimeChain = g_new0 (gchar, 100);
		strftime (cTimeChain, 100, "%F, %T", &epoch_tm);
		g_string_printf (sInfo, "<u>Last Modification</u> : %s", cTimeChain);
		g_free (cTimeChain);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		if (cMimeType != NULL)
		{
			pLabel = gtk_label_new (NULL);
			gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
			g_string_printf (sInfo, "<u>Mime Type</u> : %s", cMimeType);
			gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
			gtk_container_add (GTK_CONTAINER (pVBox), pLabel);
		}

		GtkWidget *pSeparator = gtk_hseparator_new ();
		gtk_container_add (GTK_CONTAINER (pVBox), pSeparator);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		g_string_printf (sInfo, "<u>User ID</u> : %d / <u>Group ID</u> : %d", iUID, iGID);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		int iOwnerPermissions = iPermissionsMask >> 6;  // 8*8.
		int iGroupPermissions = (iPermissionsMask - (iOwnerPermissions << 6)) >> 3;
		int iOthersPermissions = (iPermissionsMask % 8);
		g_string_printf (sInfo, "<u>Permissions</u> : read:%s / write:%s / execute:%s",
			iOwnerPermissions?"yes":"no",
			iGroupPermissions?"yes":"no",
			iOthersPermissions?"yes":"no");
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		gtk_window_set_type_hint (GTK_WINDOW (pDialog), GDK_WINDOW_TYPE_HINT_DOCK);  // sinon le dialogue sera toujours derriere le sous-dock, meme avec un 'set_keep_above'. en contrepartie, le dialgue n'aura pas de bordure (du moins avec Compiz).
		gtk_window_set_keep_above (GTK_WINDOW (pDialog), TRUE);
		
		gtk_widget_show_all (GTK_DIALOG (pDialog)->vbox);
		gtk_window_set_position (GTK_WINDOW (pDialog), GTK_WIN_POS_CENTER_ALWAYS);
		int answer = gtk_dialog_run (GTK_DIALOG (pDialog));
		gtk_widget_destroy (pDialog);

		g_string_free (sInfo, TRUE);
		g_free (cMimeType);
	}
}

static void _cd_folders_delete_file (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoContainer *pContainer = data[1];
	cd_message ("%s (%s)", __func__, icon->cName);
	
	gchar *cPath = g_filename_from_uri (icon->cBaseURI, NULL, NULL);
	g_return_if_fail (cPath != NULL);
	gchar *question = g_strdup_printf (D_("You're about deleting this file\n  (%s)\nfrom your hard-disk. Sure ?"), cPath);
	g_free (cPath);
	int answer = cairo_dock_ask_question_and_wait (question, icon, pContainer);
	g_free (question);
	if (answer == GTK_RESPONSE_YES)
	{
		gboolean bSuccess = cairo_dock_fm_delete_file (icon->cBaseURI, FALSE);
		if (! bSuccess)
		{
			cd_warning ("couldn't delete this file.\nCheck that you have writing rights on this file.\n");
			gchar *cMessage = g_strdup_printf (D_("Warning: could not delete this file.\nPlease check file permissions."));
			cairo_dock_show_temporary_dialog_with_default_icon (cMessage, icon, pContainer, 4000);
			g_free (cMessage);
		}
	}
}

static void _cd_folders_rename_file (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoContainer *pContainer = data[1];
	cd_message ("%s (%s)", __func__, icon->cName);
	
	gchar *cNewName = cairo_dock_show_demand_and_wait (D_("Rename to:"), icon, pContainer, icon->cName);
	if (cNewName != NULL && *cNewName != '\0')
	{
		gboolean bSuccess = cairo_dock_fm_rename_file (icon->cBaseURI, cNewName);
		if (! bSuccess)
		{
			cd_warning ("couldn't rename this file.\nCheck that you have writing rights, and that the new name does not already exist.");
			cairo_dock_show_temporary_dialog_with_icon_printf (D_("Warning: could not rename %s.\nCheck file permissions \nand that the new name does not already exist."), icon, pContainer, 5000, NULL, icon->cBaseURI);
		}
	}
	g_free (cNewName);
}

static void _cd_folders_move_file (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	CairoContainer *pContainer = data[1];
	CairoDockModuleInstance *myApplet = data[2];
	cd_message ("%s (%s)", __func__, icon->cName);
	
	GtkWidget* pFileChooserDialog = gtk_file_chooser_dialog_new (
		"Pick up a folder",
		GTK_WINDOW (pContainer->pWidget),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_OK,
		GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		NULL);
	
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (pFileChooserDialog), myConfig.cDirPath);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (pFileChooserDialog), FALSE);
	
	gtk_widget_show (pFileChooserDialog);
	int answer = gtk_dialog_run (GTK_DIALOG (pFileChooserDialog));
	if (answer == GTK_RESPONSE_OK)
	{
		gchar *cFilePath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (pFileChooserDialog));
		
		cairo_dock_fm_move_file (icon->cBaseURI, cFilePath);
	}
	gtk_widget_destroy (pFileChooserDialog);
}

static inline void _create_new_file (CairoDockModuleInstance *myApplet, gboolean bFolder)
{
	gchar *cNewName = cairo_dock_show_demand_and_wait (D_("Enter a file name:"), myIcon, myContainer, NULL);
	if (cNewName != NULL && *cNewName != '\0')
	{
		gchar *cURI = g_strdup_printf ("%s/%s", myConfig.cDirPath, cNewName);
		gboolean bSuccess = cairo_dock_fm_create_file (cURI, bFolder);
		if (! bSuccess)
		{
			cd_warning ("couldn't create this file.\nCheck that you have writing rights, and that the new name does not already exist.");
			cairo_dock_show_temporary_dialog_with_icon_printf (D_("Warning: could not create %s.\nCheck file permissions \nand that the new name does not already exist."), myIcon, myContainer, 5000, NULL, cNewName);
		}
	}
}
static void _cd_folders_new_file (GtkMenuItem *pMenuItem, CairoDockModuleInstance *myApplet)
{
	_create_new_file (myApplet, FALSE);
}
static void _cd_folders_new_folder (GtkMenuItem *pMenuItem, CairoDockModuleInstance *myApplet)
{
	_create_new_file (myApplet, TRUE);
}

static void _cd_folders_open_folder (GtkMenuItem *pMenuItem, CairoDockModuleInstance *myApplet)
{
	cairo_dock_fm_launch_uri (myConfig.cDirPath);
}

static void _cd_folders_launch_with (GtkMenuItem *pMenuItem, gpointer *app)
{
	Icon *icon = app[0];
	gchar *cExec = app[3];
	cairo_dock_launch_command_printf ("%s \"%s\"", NULL, cExec, icon->cBaseURI);  // en esperant que l'appli gere les URI...
}

static void _free_app (gpointer *app)
{
	g_free (app[3]);
	g_free (app);
}
void cd_folders_free_apps_list (CairoDockModuleInstance *myApplet)
{
	if (myData.pAppList != NULL)
	{
		g_list_foreach (myData.pAppList, (GFunc) _free_app, NULL);
		g_list_free (myData.pAppList);
		myData.pAppList = NULL;
	}
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	static gpointer *data = NULL;
	//g_print ("%x;%x;%x\n", icon, pContainer, menu);
	if (data == NULL)
		data = g_new0 (gpointer, 4);
	data[0] = CD_APPLET_CLICKED_ICON;
	data[1] = CD_APPLET_CLICKED_CONTAINER;
	data[2] = myApplet;
	
	if (CD_APPLET_CLICKED_ICON == myIcon || CD_APPLET_CLICKED_ICON == NULL)  // click on main icon or on the container.
	{
		GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Open the folder"), D_("middle-click"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (cLabel, GTK_STOCK_OPEN, _cd_folders_open_folder, CD_APPLET_MY_MENU, myApplet);
		g_free (cLabel);
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
	}
	else  // clic on a file.
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Rename this file"), GTK_STOCK_SAVE_AS, _cd_folders_rename_file, CD_APPLET_MY_MENU, data);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Delete this file"), GTK_STOCK_REMOVE, _cd_folders_delete_file, CD_APPLET_MY_MENU, data);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Move this file"), GTK_STOCK_JUMP_TO, _cd_folders_move_file, CD_APPLET_MY_MENU, data);
		
		GList *pApps = cairo_dock_fm_list_apps_for_file (CD_APPLET_CLICKED_ICON->cBaseURI);
		if (pApps != NULL)
		{
			CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
			GtkWidget *pSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Open with"), CD_APPLET_MY_MENU, GTK_STOCK_OPEN);
			
			cd_folders_free_apps_list (myApplet);
			
			GList *a;
			gchar **pAppInfo;
			gchar *cIconPath;
			gpointer *app;
			for (a = pApps; a != NULL; a = a->next)
			{
				pAppInfo = a->data;
				
				app = g_new0 (gpointer, 4);
				app[0] = CD_APPLET_CLICKED_ICON;
				app[1] = CD_APPLET_CLICKED_CONTAINER;
				app[2] = myApplet;
				app[3] = g_strdup (pAppInfo[1]);
				//g_print (" + %s (%s ; %s)\n", pAppInfo[0], pAppInfo[1], pAppInfo[2]);
				myData.pAppList = g_list_prepend (myData.pAppList, app);
				
				if (pAppInfo[2] != NULL)
					cIconPath = cairo_dock_search_icon_s_path (pAppInfo[2]);
				else
					cIconPath = NULL;
				CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (pAppInfo[0], cIconPath, _cd_folders_launch_with, pSubMenu, app);
				g_free (cIconPath);
				g_strfreev (pAppInfo);
			}
			g_list_free (pApps);
		}
		
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Properties"), GTK_STOCK_PROPERTIES, _cd_folders_show_file_properties, CD_APPLET_MY_MENU, data);
		
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Create a new file"), GTK_STOCK_NEW, _cd_folders_new_file, CD_APPLET_MY_MENU, myApplet);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Create a new folder"), GTK_STOCK_NEW, _cd_folders_new_folder, CD_APPLET_MY_MENU, myApplet);
	}
	
	if (CD_APPLET_CLICKED_ICON != NULL && CD_APPLET_CLICKED_ICON != myIcon)
		CD_APPLET_LEAVE (CAIRO_DOCK_INTERCEPT_NOTIFICATION);
CD_APPLET_ON_BUILD_MENU_END


gboolean cd_folders_on_drop_data (gpointer data, const gchar *cReceivedData, Icon *icon, double fOrder, CairoContainer *pContainer)
{
	//g_print ("Folders received '%s'\n", cReceivedData);
	
	if (fOrder == CAIRO_DOCK_LAST_ORDER)  // lachage sur une icone.
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	gchar *cPath = NULL;
	if (strncmp (cReceivedData, "file://", 7) == 0)  // tous les programmes ne gerent pas les URI; pour parer au cas ou il ne le gererait pas, dans le cas d'un fichier local, on convertit en un chemin
		cPath = g_filename_from_uri (cReceivedData, NULL, NULL);
	else
		cPath = g_strdup (cReceivedData);
	
	if (g_file_test (cPath, G_FILE_TEST_IS_DIR))
	{
		//g_print (" ajout d'un repertoire...\n");
		CairoDockModule *pModule = cairo_dock_find_module_from_name ("Folders");
		g_return_val_if_fail (pModule != NULL, CAIRO_DOCK_LET_PASS_NOTIFICATION);
		
		gchar *cConfFilePath = cairo_dock_add_module_conf_file (pModule);
		cairo_dock_update_conf_file (cConfFilePath,
			G_TYPE_STRING, "Configuration", "dir path", cReceivedData,
			G_TYPE_INVALID);
		
		CairoDockModuleInstance *pNewInstance = cairo_dock_instanciate_module (pModule, cConfFilePath);  // prend le 'cConfFilePath'.
		if (pNewInstance != NULL && pNewInstance->pDock)
		{
			cairo_dock_update_dock_size (pNewInstance->pDock);
		}
		
		if (pModule->pInstancesList && pModule->pInstancesList->next == NULL)  // module nouvellement active.
		{
			cairo_dock_write_active_modules ();
		}
		
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
