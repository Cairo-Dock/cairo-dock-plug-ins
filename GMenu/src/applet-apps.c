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

static gboolean s_bDesktopEnvDef = TRUE; // can we use g_app_info_should_show()?

void cd_menu_register_app (GDesktopAppInfo *pAppInfo)
{
	const gchar *cDesktopFilePath = g_desktop_app_info_get_filename (pAppInfo);
	g_return_if_fail (cDesktopFilePath != NULL);
	
	if (! g_hash_table_lookup (myData.pKnownApplications, cDesktopFilePath))  // do it the first time too, to avoid doubles
	{
		if (!myData.bFirstLaunch)
		{
			myData.pNewApps = g_list_prepend (myData.pNewApps, pAppInfo);
		}
		g_hash_table_insert (myData.pKnownApplications, g_strdup (cDesktopFilePath), g_object_ref (pAppInfo));  // since the info won't change for a given app, no need to re-insert; so we ref the appinfo, and keep it alive.
		myData.pApps = g_slist_prepend (myData.pApps, pAppInfo);
	}
}

/**
 * Returns true if the app should be shown in the menu
 */
gboolean cd_menu_app_should_show (GDesktopAppInfo *pAppInfo)
{
	// should_show = NoDisplay + OnlyShowIn
	if (s_bDesktopEnvDef)
		return g_app_info_should_show (G_APP_INFO (pAppInfo));

	// XDG_CURRENT_DESKTOP is not defined => only check 'NoDisplay', if available
	#if GLIB_CHECK_VERSION (2, 30, 0)
	return ! g_desktop_app_info_get_nodisplay (pAppInfo);
	#else
	return TRUE;
	#endif
}

static CairoDialog *s_pNewAppsDialog = NULL;

static void _on_answer_launch_recent (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok ou entree.
	{
		GDesktopAppInfo *pAppInfo = NULL;
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
		cairo_dock_launch_app_info (pAppInfo);
	}
	
	g_list_free (myData.pNewApps);  // the content elongs to pKnownApplications
	myData.pNewApps = NULL;
	s_pNewAppsDialog = NULL;
}

#ifdef END_INSTALLATION_PID
static gboolean _show_new_apps_dialog_idle (gpointer pData)
{
	if (pData)
		gldi_dialog_unhide (pData);
	return FALSE;
}
#endif

void cd_menu_check_for_new_apps (void)
{
	if (myData.pNewApps != NULL && myConfig.bShowNewApps)
	{
		if (s_pNewAppsDialog) // the dialogue already exists: add new items in the list
		{
			gtk_combo_box_text_remove_all (
				GTK_COMBO_BOX_TEXT (s_pNewAppsDialog->pInteractiveWidget));
			GList *a;
			for (a = myData.pNewApps; a != NULL; a = a->next)
			{
				gtk_combo_box_text_append_text (
					GTK_COMBO_BOX_TEXT (s_pNewAppsDialog->pInteractiveWidget),
					g_app_info_get_name (G_APP_INFO (a->data)));
			}
			gtk_combo_box_set_active (
				GTK_COMBO_BOX (s_pNewAppsDialog->pInteractiveWidget), 0);

			gldi_dialog_redraw_interactive_widget (s_pNewAppsDialog);
		}
		else
		{
			const gchar *cQuestion = D_("Launch this new application?");
			GtkWidget *pInteractiveWidget = gtk_combo_box_text_new ();
			GList *a;
			for (a = myData.pNewApps; a != NULL; a = a->next)
			{
				gtk_combo_box_text_append_text (
					GTK_COMBO_BOX_TEXT (pInteractiveWidget),
					g_app_info_get_name (G_APP_INFO (a->data)));
			}

			gtk_combo_box_set_active (GTK_COMBO_BOX (pInteractiveWidget), 0); // select the first one

			gchar *cIconPath = cairo_dock_search_icon_s_path (GLDI_ICON_NAME_EXECUTE, myDialogsParam.iDialogIconSize);
			s_pNewAppsDialog = gldi_dialog_show (cQuestion,
				myIcon, myContainer,
				0,
				cIconPath ? cIconPath : "same icon",
				pInteractiveWidget, (CairoDockActionOnAnswerFunc)_on_answer_launch_recent,
				NULL,
				(GFreeFunc)NULL);
			#ifdef END_INSTALLATION_PID
			gldi_dialog_hide (s_pNewAppsDialog);
			cairo_dock_fm_monitor_pid (END_INSTALLATION_PID, FALSE,
					_show_new_apps_dialog_idle, TRUE, s_pNewAppsDialog);
			#endif
			g_free (cIconPath);
		}
	}
	myData.bFirstLaunch = FALSE;
}


void cd_menu_free_apps (void)
{
	g_hash_table_destroy (myData.pKnownApplications);
	g_slist_free (myData.pApps);
	g_list_free (myData.pNewApps);
}


void cd_menu_init_apps (void)
{
	if (! myData.pKnownApplications)  // only init once
	{
		// check the XDG_CURRENT_DESKTOP env variable, it is used to match the 'OnlyShowIn' key in the .desktop files in gio and libgnomemenu
		const gchar *cDesktopEnv = g_getenv ("XDG_CURRENT_DESKTOP");
		if (! cDesktopEnv)  // the system didn't set the variable: this is probably a bug, so let's try to set a value ourselves (this is needed for both gio and libgnomemenu); this is a workaround, most of the time the value will be set correctly.
		{
			switch (g_iDesktopEnv)
			{
				case CAIRO_DOCK_GNOME: cDesktopEnv = "GNOME"; break;
				case CAIRO_DOCK_XFCE: cDesktopEnv = "XFCE"; break;
				case CAIRO_DOCK_KDE: cDesktopEnv = "KDE"; break;
				default: break;
			}
			// if we found something, set the env variable, because libgnomemenu uses it directly, and if it is not set, will drop any element that has a OnlyShowIn key...
			if (cDesktopEnv)
				g_setenv ("XDG_CURRENT_DESKTOP", cDesktopEnv, TRUE);
		}

		#if ! GLIB_CHECK_VERSION (2,41,0)
		// Since 2.41 the value of the XDG_CURRENT_DESKTOP environment variable will be used.
		if (cDesktopEnv)  // also set it for g_app_info_should_show() in gio
			g_desktop_app_info_set_desktop_env (cDesktopEnv);
		#endif

		s_bDesktopEnvDef = (cDesktopEnv != NULL);  // if cDesktopEnv is NULL, g_app_info_should_show will drop any element that has a OnlyShowIn key (it does a direct comparison), so we won't use this function, as it's better to show more apps than having missing apps.

		myData.bFirstLaunch = TRUE;
		myData.pKnownApplications = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free,
			g_object_unref);
	}
}

