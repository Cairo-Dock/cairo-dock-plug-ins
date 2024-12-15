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
#include <time.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-notifications.h"


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	if (CD_APPLET_CLICKED_ICON == myIcon)
	{
		if (! myConfig.bShowFiles)
		{
			cairo_dock_fm_launch_uri (myConfig.cDirPath);
		}
		else if (CD_APPLET_MY_ICONS_LIST == NULL)  // repertoire vide, ou illisible, ou non defini
		{
			gldi_dialogs_remove_on_icon (myIcon);
			if (myConfig.cDirPath == NULL)
				gldi_dialog_show_temporary_with_icon (D_("Open the configuration of the applet to choose a folder to import."),
					myIcon, myContainer,
					8000.,
					myConfig.iSubdockViewType == 0 ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
			else
			{
				gchar *cPath = g_filename_from_uri (myConfig.cDirPath, NULL, NULL);
				gldi_dialog_show_temporary_with_icon_printf ("%s :\n%s",
					myIcon, myContainer,
					4000.,
					myConfig.iSubdockViewType == 0 ? "same icon" : MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
					cPath ? cPath : myConfig.cDirPath,
					D_("Empty or unreadable folder."));
				g_free (cPath);
			}
		}
		else
			CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);  // on laisse passer la notification (pour ouvrir le sous-dock au clic).
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

#define MARGIN 3
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
			D_("Properties:"));

		GString *sInfo = g_string_new ("");
		g_string_printf (sInfo, "<b>%s</b>", icon->cName);

		GtkWidget *pLabel= gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);

		GtkWidget *pFrame = gtk_frame_new (NULL);
		gtk_container_set_border_width (GTK_CONTAINER (pFrame), MARGIN);
		gtk_frame_set_label_widget (GTK_FRAME (pFrame), pLabel);
		gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG(pDialog))), pFrame);
		gtk_frame_set_shadow_type (GTK_FRAME (pFrame), GTK_SHADOW_OUT);

		GtkWidget *pVBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN);
		gtk_container_add (GTK_CONTAINER (pFrame), pVBox);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		g_string_printf (sInfo, "<u>%s</u>: %"G_GUINT64_FORMAT" %s", D_("Size"), iSize, D_("bytes"));
		if (iSize > 1024*1024)
			g_string_append_printf (sInfo, " (%.1f %s)", 1. * iSize / 1024 / 1024, D_("MB"));
		else if (iSize > 1024)
			g_string_append_printf (sInfo, " (%.1f %s)", 1. * iSize / 1024, D_("KB"));
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		struct tm epoch_tm;
		localtime_r (&iLastModificationTime, &epoch_tm);  // et non pas gmtime_r.
		gchar *cTimeChain = g_new0 (gchar, 100);
		strftime (cTimeChain, 100, "%F, %T", &epoch_tm);
		g_string_printf (sInfo, "<u>%s</u>: %s", D_("Last Modification"), cTimeChain);
		g_free (cTimeChain);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		if (cMimeType != NULL)
		{
			pLabel = gtk_label_new (NULL);
			gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
			g_string_printf (sInfo, "<u>%s</u>: %s", D_("Mime Type"), cMimeType);
			gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
			gtk_container_add (GTK_CONTAINER (pVBox), pLabel);
		}

		GtkWidget *pSeparator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		gtk_container_add (GTK_CONTAINER (pVBox), pSeparator);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		g_string_printf (sInfo, "<u>%s</u>: %d \t <u>%s</u>: %d", D_("User ID"), iUID, D_("Group ID"), iGID);
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		pLabel = gtk_label_new (NULL);
		gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
		int iOwnerPermissions = iPermissionsMask >> 6;  // 8*8.
		int iGroupPermissions = (iPermissionsMask - (iOwnerPermissions << 6)) >> 3;
		int iOthersPermissions = (iPermissionsMask % 8);
		g_string_printf (sInfo, "<u>%s</u>: %s: %s / %s: %s / %s: %s",
			D_("Permissions"), D_("Read"),
			iOwnerPermissions ? D_("Yes") : D_("No"), D_("Write"),
			iGroupPermissions ? D_("Yes") : D_("No"), D_("Execute"),
			iOthersPermissions ? D_("Yes") : D_("No"));
		gtk_label_set_markup (GTK_LABEL (pLabel), sInfo->str);
		gtk_container_add (GTK_CONTAINER (pVBox), pLabel);

		gtk_window_set_type_hint (GTK_WINDOW (pDialog), GDK_WINDOW_TYPE_HINT_DOCK);  // sinon le dialogue sera toujours derriere le sous-dock, meme avec un 'set_keep_above'. en contrepartie, le dialgue n'aura pas de bordure (du moins avec Compiz).
		gtk_window_set_keep_above (GTK_WINDOW (pDialog), TRUE);
		
		gtk_widget_show_all (gtk_dialog_get_content_area (GTK_DIALOG(pDialog)));
		gtk_window_set_position (GTK_WINDOW (pDialog), GTK_WIN_POS_CENTER_ALWAYS);
		gtk_dialog_run (GTK_DIALOG (pDialog));
		gtk_widget_destroy (pDialog);

		g_string_free (sInfo, TRUE);
		g_free (cMimeType);
	}
}

static void _on_answer_delete_file (int iClickedButton, GtkWidget *pInteractiveWidget, Icon *icon, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		gboolean bSuccess = cairo_dock_fm_delete_file (icon->cBaseURI, FALSE);
		if (! bSuccess)
		{
			cd_warning ("couldn't delete this file.\nCheck that you have writing rights on this file.\n");
			gchar *cMessage = g_strdup_printf (D_("Warning: could not delete this file.\nPlease check file permissions."));
			gldi_dialog_show_temporary_with_default_icon (cMessage, icon, icon->pContainer, 4000);
			g_free (cMessage);
		}
	}
}
static void _cd_folders_delete_file (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	GldiContainer *pContainer = data[1];
	cd_message ("%s (%s)", __func__, icon->cName);
	
	gchar *cPath = g_filename_from_uri (icon->cBaseURI, NULL, NULL);
	g_return_if_fail (cPath != NULL);
	gchar *question = g_strdup_printf (D_("You're about deleting this file\n  (%s)\nfrom your hard-disk. Sure ?"), cPath);
	g_free (cPath);
	gldi_dialog_show_with_question (question,
		icon, pContainer,
		"same icon",
		(CairoDockActionOnAnswerFunc) _on_answer_delete_file, icon, (GFreeFunc)NULL);  // if the icon is deleted during the question, the dialog will disappear with it, and we won't be called back.
}

static void _on_answer_rename_file (int iClickedButton, GtkWidget *pInteractiveWidget, Icon *icon, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		const gchar *cNewName = gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget));
		if (cNewName != NULL && *cNewName != '\0')
		{
			gboolean bSuccess = cairo_dock_fm_rename_file (icon->cBaseURI, cNewName);
			if (! bSuccess)
			{
				cd_warning ("couldn't rename this file.\nCheck that you have writing rights, and that the new name does not already exist.");
				gldi_dialog_show_temporary_with_icon_printf (D_("Warning: could not rename %s.\nCheck file permissions \nand that the new name does not already exist."), icon, icon->pContainer, 5000, NULL, icon->cBaseURI);
			}
		}
	}
}
static void _cd_folders_rename_file (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	cd_message ("%s (%s)", __func__, icon->cName);
	gldi_dialog_show_with_entry (D_("Rename to:"),
		icon, icon->pContainer,
		"same icon",
		icon->cName,
		(CairoDockActionOnAnswerFunc)_on_answer_rename_file, icon, (GFreeFunc)NULL);
}

static void _cd_folders_move_file (GtkMenuItem *pMenuItem, gpointer *data)
{
	Icon *icon = data[0];
	GldiContainer *pContainer = data[1];
	GldiModuleInstance *myApplet = data[2];
	cd_message ("%s (%s)", __func__, icon->cName);
	
	GtkWidget* pFileChooserDialog = gtk_file_chooser_dialog_new (
		_("Pick up a directory"),
		GTK_WINDOW (pContainer->pWidget),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		_("Ok"),
		GTK_RESPONSE_OK,
		_("Cancel"),
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

static void _on_answer_create_file (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer *data, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		gboolean bFolder = GPOINTER_TO_INT (data[0]);
		GldiModuleInstance *myApplet = data[1];
		const gchar *cNewName = gtk_entry_get_text (GTK_ENTRY (pInteractiveWidget));
		if (cNewName != NULL && *cNewName != '\0')
		{
			gchar *cURI = g_strdup_printf ("%s/%s", myConfig.cDirPath, cNewName);
			gboolean bSuccess = cairo_dock_fm_create_file (cURI, bFolder);
			if (! bSuccess)
			{
				cd_warning ("couldn't create this file.\nCheck that you have writing rights, and that the new name does not already exist.");
				gldi_dialog_show_temporary_with_icon_printf (D_("Warning: could not create %s.\nCheck file permissions \nand that the new name does not already exist."), myIcon, myContainer, 5000, NULL, cNewName);
			}
		}
	}
}
static inline void _create_new_file (GldiModuleInstance *myApplet, gboolean bFolder)
{
	gpointer *data = g_new (gpointer, 2);
	data[0] = GINT_TO_POINTER (bFolder);
	data[1] = myApplet;
	gldi_dialog_show_with_entry (D_("Enter a file name:"),
		myIcon, myContainer,
		"same icon",
		NULL,
		(CairoDockActionOnAnswerFunc)_on_answer_create_file, data, (GFreeFunc)g_free);
}
static void _cd_folders_new_file (GtkMenuItem *pMenuItem, GldiModuleInstance *myApplet)
{
	_create_new_file (myApplet, FALSE);
}
static void _cd_folders_new_folder (GtkMenuItem *pMenuItem, GldiModuleInstance *myApplet)
{
	_create_new_file (myApplet, TRUE);
}

static void _cd_folders_open_folder (GtkMenuItem *pMenuItem, GldiModuleInstance *myApplet)
{
	cairo_dock_fm_launch_uri (myConfig.cDirPath);
}

static void _cd_folders_sort_by_name (GtkMenuItem *pMenuItem, GldiModuleInstance *myApplet)
{
	cd_folders_sort_icons (myApplet, CAIRO_DOCK_FM_SORT_BY_NAME);
}

static void _cd_folders_sort_by_date (GtkMenuItem *pMenuItem, GldiModuleInstance *myApplet)
{
	cd_folders_sort_icons (myApplet, CAIRO_DOCK_FM_SORT_BY_DATE);
}

static void _cd_folders_sort_by_size (GtkMenuItem *pMenuItem, GldiModuleInstance *myApplet)
{
	cd_folders_sort_icons (myApplet, CAIRO_DOCK_FM_SORT_BY_SIZE);
}

static void _cd_folders_sort_by_type (GtkMenuItem *pMenuItem, GldiModuleInstance *myApplet)
{
	cd_folders_sort_icons (myApplet, CAIRO_DOCK_FM_SORT_BY_TYPE);
}

static void _free_app (gpointer *app)
{
	g_free (app[3]);
	g_free (app);
}
void cd_folders_free_apps_list (GldiModuleInstance *myApplet)
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
		if (myConfig.bShowFiles)
		{
			gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Open the folder"), D_("middle-click"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (cLabel, GLDI_ICON_NAME_OPEN, _cd_folders_open_folder, CD_APPLET_MY_MENU, myApplet);
			g_free (cLabel);
		}
	}
	else  // clic on a file.
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Rename this file"), GLDI_ICON_NAME_SAVE_AS, _cd_folders_rename_file, CD_APPLET_MY_MENU, data);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Delete this file"), GLDI_ICON_NAME_REMOVE, _cd_folders_delete_file, CD_APPLET_MY_MENU, data);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Move this file"), GLDI_ICON_NAME_JUMP_TO, _cd_folders_move_file, CD_APPLET_MY_MENU, data);
		
		GList *pApps = cairo_dock_fm_list_apps_for_file (CD_APPLET_CLICKED_ICON->cBaseURI);
		if (pApps != NULL)
		{
			CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
			cairo_dock_fm_add_open_with_submenu (pApps, CD_APPLET_CLICKED_ICON->cBaseURI, CD_APPLET_MY_MENU,
				D_("Open with"), GLDI_ICON_NAME_OPEN, NULL, NULL);
			g_list_free_full (pApps, g_object_unref);
		}
		
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Properties"), GLDI_ICON_NAME_PROPERTIES, _cd_folders_show_file_properties, CD_APPLET_MY_MENU, data);
		
		CD_APPLET_ADD_SEPARATOR_IN_MENU (CD_APPLET_MY_MENU);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Create a new file"), GLDI_ICON_NAME_NEW, _cd_folders_new_file, CD_APPLET_MY_MENU, myApplet);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Create a new folder"), GLDI_ICON_NAME_NEW, _cd_folders_new_folder, CD_APPLET_MY_MENU, myApplet);
	}
	
	if (myConfig.bShowFiles)
	{
		GtkWidget *pSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Sort by"), CD_APPLET_MY_MENU, GLDI_ICON_NAME_SORT_DESCENDING);
		pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("By name"), _cd_folders_sort_by_name, pSubMenu, myApplet);
		pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("By date"), _cd_folders_sort_by_date, pSubMenu, myApplet);
		pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("By size"), _cd_folders_sort_by_size, pSubMenu, myApplet);
		pMenuItem = CD_APPLET_ADD_IN_MENU_WITH_DATA (D_("By type"), _cd_folders_sort_by_type, pSubMenu, myApplet);
	}
	
	if (CD_APPLET_CLICKED_ICON != NULL && CD_APPLET_CLICKED_ICON != myIcon)
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_INTERCEPT);
CD_APPLET_ON_BUILD_MENU_END


typedef struct {
	gchar *cReceivedData;
	double fOrder;
	gchar *cDockName;
} CDDropData;

static void _on_answer_import (int iClickedButton, GtkWidget *pInteractiveWidget, CDDropData *data, CairoDialog *pDialog)
{
	cd_debug ("%s (%d)", __func__, iClickedButton);
	const gchar *cReceivedData = data->cReceivedData;
	double fOrder = data->fOrder;
	gboolean bImportFiles = (iClickedButton == 0 || iClickedButton == -1);  // ok or Enter.
	
	// add a new conf file for the "Folders" module, with proper values.
	GldiModule *pModule = gldi_module_get ("Folders");
	g_return_if_fail (pModule != NULL);

	gchar *cConfFilePath = gldi_module_add_conf_file (pModule);  // we want to update the conf file before we instanciate the applet, so don't use high-level functions.
	cairo_dock_update_conf_file (cConfFilePath,
		G_TYPE_STRING, "Configuration", "dir path", cReceivedData,
		G_TYPE_BOOLEAN, "Configuration", "show files", bImportFiles,
		G_TYPE_DOUBLE, "Icon", "order", fOrder,
		G_TYPE_STRING, "Icon", "dock name", data->cDockName,
		G_TYPE_INVALID);
	
	// instanciate the module from this conf file.
	GldiModuleInstance *pNewInstance = gldi_module_instance_new (pModule, cConfFilePath);  // prend le 'cConfFilePath'.
	
	// show a success message on the new icon.
	if (pNewInstance != NULL)
		gldi_dialog_show_temporary_with_icon (D_("The folder has been imported."),
			pNewInstance->pIcon, pNewInstance->pContainer,
			5000,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);  // not "same icon" because the icon may not be loaded yet (eg. stack or emblem icon).
}
static void _free_dialog_data (CDDropData *data)
{
	g_free (data->cReceivedData);
	g_free (data->cDockName);
	g_free (data);
}
gboolean cd_folders_on_drop_data (gpointer data, const gchar *cReceivedData, Icon *icon, double fOrder, GldiContainer *pContainer)
{
	//g_print ("Folders received '%s'\n", cReceivedData);
	if (icon != NULL || fOrder == CAIRO_DOCK_LAST_ORDER)  // drop on an icon or outside of icons.
		return GLDI_NOTIFICATION_LET_PASS;
	
	gchar *cPath = NULL;
	if (strncmp (cReceivedData, "file://", 7) == 0)
		cPath = g_filename_from_uri (cReceivedData, NULL, NULL);
	else
		cPath = g_strdup (cReceivedData);
	
	if (g_file_test (cPath, G_FILE_TEST_IS_DIR))  // it's a folder, let's add a new instance of the applet that will handle it.
	{
		// search the closest icon to the drop point (we want to place the dialog on it).
		GList *pIconsList = NULL, *ic;
		if (CAIRO_DOCK_IS_DOCK (pContainer))
			pIconsList = CAIRO_DOCK (pContainer)->icons;
		else if (CAIRO_DOCK_IS_DESKLET (pContainer))
			pIconsList = CAIRO_DESKLET (pContainer)->icons;
		Icon *pIcon = NULL;
		for (ic = pIconsList; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			if (icon->fOrder > fOrder)
			{
				pIcon = icon;
				break;
			}
		}
		if (pIcon == NULL)
		{
			if (CAIRO_DOCK_IS_DOCK (pContainer))
				pIcon = gldi_icons_get_without_dialog (CAIRO_DOCK (pContainer)->icons);
			else
				pIcon = gldi_icons_get_any_without_dialog ();
		}
		
		// ask the user whether (s)he wants to import the folder's content.
		CDDropData *data = g_new0 (CDDropData, 1);
		data->cReceivedData = g_strdup (cReceivedData);
		data->fOrder = fOrder;
		if (CAIRO_DOCK_IS_DOCK (pContainer))
			data->cDockName = g_strdup (gldi_dock_get_name (CAIRO_DOCK (pContainer)));
		gldi_dialog_show (D_("Do you want to import the content of the folder too?"),
			pIcon, pContainer,
			0,
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
			NULL,
			(CairoDockActionOnAnswerFunc) _on_answer_import,
			data,
			(GFreeFunc)_free_dialog_data);
		
		return GLDI_NOTIFICATION_INTERCEPT;
	}
	return GLDI_NOTIFICATION_LET_PASS;
}
