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

#include "applet-struct.h"
#include "applet-apps.h"


void cd_menu_register_app (GDesktopAppInfo *pAppInfo)
{
	const gchar *cDesktopFilePath = g_desktop_app_info_get_filename (pAppInfo);
	g_return_if_fail (cDesktopFilePath != NULL);
	
	if (! g_hash_table_lookup (myData.pKnownApplications, cDesktopFilePath))  // do it the first time too, to avoid doubles
	{
		if (!myData.bFirstLaunch)
		{
			g_print ("+++ %s is NEW\n", cDesktopFilePath);
			myData.pNewApps = g_list_prepend (myData.pNewApps, pAppInfo);
		}
		g_hash_table_insert (myData.pKnownApplications, g_strdup (cDesktopFilePath), g_object_ref (pAppInfo));  // since the info won't change for a given app, no need to re-insert; so we ref the appinfo, and keep it alive.
	}
}


static void _on_answer_launch_recent (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok ou entree.
	{
		GAppInfo *pAppInfo = NULL;
		if (pInteractiveWidget)  // find the selected one
		{
			int n = gtk_combo_box_get_active (GTK_COMBO_BOX (pInteractiveWidget));
			pAppInfo = g_list_nth_data (myData.pNewApps, n);
		}
		else
		{
			pAppInfo = myData.pNewApps->data;
		}
		g_return_if_fail (pAppInfo != NULL);
		g_app_info_launch (pAppInfo, NULL, NULL, NULL);
	}
	
	g_list_free (myData.pNewApps);  // the content elongs to pKnownApplications
	myData.pNewApps = NULL;
}

void cd_menu_check_for_new_apps (void)
{
	if (myData.pNewApps != NULL)
	{
		const gchar *cQuestion = D_("Launch this new application?");
		gchar *cText = NULL;
		GtkWidget *pInteractiveWidget = NULL;
		if (myData.pNewApps->next)  // several entries, make a list
		{
			pInteractiveWidget = gtk_combo_box_text_new ();
			GList *a;
			for (a = myData.pNewApps; a != NULL; a = a->next)
			{
				gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pInteractiveWidget), g_app_info_get_name (a->data));
			}
		}
		else
		{
			cText = g_strconcat (cQuestion, "\n  ", g_app_info_get_display_name (G_APP_INFO (myData.pNewApps->data)), NULL);
		}
		gchar *cIconPath = cairo_dock_search_icon_s_path (GTK_STOCK_EXECUTE, myDialogsParam.iDialogIconSize);
		gldi_dialog_show (cText?cText:cQuestion,
			myIcon, myContainer,
			0,
			cIconPath ? cIconPath : "same icon",
			pInteractiveWidget, (CairoDockActionOnAnswerFunc)_on_answer_launch_recent,
			NULL,
			(GFreeFunc)NULL);
		g_free (cIconPath);
		g_free (cText);
	}
	myData.bFirstLaunch = FALSE;
}


void cd_menu_free_apps (void)
{
	g_hash_table_destroy (myData.pKnownApplications);
}


void cd_menu_init_apps (void)
{
	if (! myData.pKnownApplications)
	{
		myData.bFirstLaunch = TRUE;
		myData.pKnownApplications = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free,
			g_object_unref);
	}
}

