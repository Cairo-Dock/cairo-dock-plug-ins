/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

extern gchar **my_cTrashDirectoryList;
extern gchar *my_cDefaultBrowser;
extern int my_iNbTrash;


CD_APPLET_ABOUT (_D("This is the dustbin applet for Cairo-Dock\n made by Fabrice Rey (fabounet@users.berlios.de)"))


CD_APPLET_ON_CLICK_BEGIN
	g_print ("_Note_ : You can manage many Trash directories with this applet.\n Right click on its icon to see which Trash directories are already being monitored.\n");
	cd_dustbin_show_trash (NULL, "trash:/");  // my_cTrashDirectoryList[0]
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Dustbin", pModuleSubMenu, CD_APPLET_MY_MENU)
	int i = 0;
	if (my_cTrashDirectoryList != NULL)
	{
		while (my_cTrashDirectoryList[i] != NULL)
			i ++;
	}
	my_iNbTrash = i;
	
	GString *sLabel = g_string_new ("");
	
	CD_APPLET_ADD_SUB_MENU (_D("Show Trash"), pShowSubMenu, pModuleSubMenu)
	i = 0;
	if (my_cTrashDirectoryList != NULL)
	{
		while (my_cTrashDirectoryList[i] != NULL)
		{
			g_string_printf (sLabel, _D("Show %s"), my_cTrashDirectoryList[i]);
			
			CD_APPLET_ADD_IN_MENU_WITH_DATA (sLabel->str, cd_dustbin_show_trash, pShowSubMenu, my_cTrashDirectoryList[i])
			i ++;
		}
	}
	CD_APPLET_ADD_IN_MENU (_D("Show All"), cd_dustbin_show_trash, pShowSubMenu)
	
	CD_APPLET_ADD_SUB_MENU (_D("Delete Trash"), pDeleteSubMenu, pModuleSubMenu)
	i = 0;
	if (my_cTrashDirectoryList != NULL)
	{
		while (my_cTrashDirectoryList[i] != NULL)
		{
			g_string_printf (sLabel, _D("Delete %s"), my_cTrashDirectoryList[i]);
			
			CD_APPLET_ADD_IN_MENU_WITH_DATA (sLabel->str, cd_dustbin_delete_trash, pDeleteSubMenu, my_cTrashDirectoryList[i])
			
			i ++;
		}
	}
	CD_APPLET_ADD_IN_MENU (_D("Delete All"), cd_dustbin_delete_trash, pDeleteSubMenu)
	
	g_string_free (sLabel, TRUE);
	
	CD_APPLET_ADD_ABOUT_IN_MENU (pModuleSubMenu)
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_DROP_DATA_BEGIN
	g_print ("  %s --> a la poubelle !\n", CD_APPLET_RECEIVED_DATA);
	gchar *cName=NULL, *cURI=NULL, *cIconName=NULL;
	gboolean bIsDirectory;
	int iVolumeID = 0;
	double fOrder;
	/*if (strncmp (CD_APPLET_RECEIVED_DATA, "x-nautilus-desktop://", 21) == 0)
	{
		gchar *cNautilusFile = g_strdup_printf ("computer://%s", CD_APPLET_RECEIVED_DATA+21);
		if (g_str_has_suffix ( CD_APPLET_RECEIVED_DATA, ".volume"))
		{
			cNautilusFile[strlen(cNautilusFile)-6] = '\0';
			gchar *cNautilusDrive = g_strdup_printf (cNautilusFile, ".drive");
			g_free (cNautilusFile);
			g_print ("cNautilusDrive : %s\n", cNautilusDrive);
			cairo_dock_fm_unmount_full (cNautilusDrive, 0, cairo_dock_fm_action_after_mounting, myIcon, myDock);
		}
	}*/
	///x-nautilus-desktop:///Lecteur%20de%20musique%20Samsung%20YP-U2.volume
	///computer:///Lecteur%2520de%2520musique%2520Samsung%2520YP-U2.drive
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
			cairo_dock_fm_unmount_full (cURI, iVolumeID, cairo_dock_fm_action_after_mounting, myIcon, myDock);
		else
			cairo_dock_fm_move_file (cURI, "trash:///");
			//cairo_dock_fm_delete_file (cURI);
	}
	else
	{
		gchar *cHostname = NULL;
		GError *erreur = NULL;
		gchar *cFileName = g_filename_from_uri (CD_APPLET_RECEIVED_DATA, &cHostname, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : can't find valid URI for '%s' : %s\n", CD_APPLET_RECEIVED_DATA, erreur->message);
			g_error_free (erreur);
		}
		else if ((cHostname == NULL || strcmp (cHostname, "localhost") == 0) && my_cTrashDirectoryList != NULL)
		{
			gchar *cCommand = g_strdup_printf ("mv %s %s", cFileName,my_cTrashDirectoryList[0]);
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


void cd_dustbin_delete_trash (GtkMenuItem *menu_item, gchar *cDirectory)
{
	gchar *question;
	if (cDirectory != NULL)
		question = g_strdup_printf (_D("You're about to delete all files in %s. Sure ?"), cDirectory);
	else if (my_cTrashDirectoryList != NULL)
		question = g_strdup_printf (_D("You're about to delete all files in all dustbins. Sure ?"));
	else
		return;
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		question);
	g_free (question);
	int answer = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (answer == GTK_RESPONSE_YES)
	{
		GString *sCommand = g_string_new ("rm -rf ");
		if (cDirectory != NULL)
		{
			g_string_append_printf (sCommand, "%s/*", cDirectory);
		}
		else
		{
			int i = 0;
			while (my_cTrashDirectoryList[i] != NULL)
			{
				g_string_append_printf (sCommand, "%s ", my_cTrashDirectoryList[i]);
				i ++;
			}
		}
		g_print (">>> %s\n", sCommand->str);
		GError *erreur = NULL;
		g_spawn_command_line_async (sCommand->str, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : when trying to execute '%s' : %s\n", sCommand->str, erreur->message);
			g_error_free (erreur);
		}
		g_string_free (sCommand, TRUE);
	}
}


void cd_dustbin_show_trash (GtkMenuItem *menu_item, gchar *cDirectory)
{
	if (my_cDefaultBrowser != NULL)
	{
		GString *sCommand = g_string_new (my_cDefaultBrowser);
		if (cDirectory != NULL)
		{
			g_string_append_printf (sCommand, " %s", cDirectory);
		}
		else
		{
			int i = 0;
			while (my_cTrashDirectoryList[i] != NULL)
			{
				g_string_append_printf (sCommand, " %s", my_cTrashDirectoryList[i]);
				i ++;
			}
		}
		//g_print (">>> %s\n", sCommand->str);
		GError *erreur = NULL;
		g_spawn_command_line_async (sCommand->str, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : when trying to execute '%s' : %s\n", sCommand->str, erreur->message);
			g_error_free (erreur);
			//gchar *cTipMessage = g_strdup_printf ("A problem occured\nIf '%s' is not your usual file browser, you can change it in the conf panel of this module", my_cDefaultBrowser);
			cairo_dock_show_temporary_dialog (_D("A problem occured\nIf '%s' is not your usual file browser,\nyou can change it in the conf panel of this module"), myIcon, myDock, 5000, my_cDefaultBrowser);
			//g_free (cTipMessage);
		}
		g_string_free (sCommand, TRUE);
	}
	else if (cDirectory != NULL)
	{
		cairo_dock_fm_launch_uri (cDirectory);
	}
}
