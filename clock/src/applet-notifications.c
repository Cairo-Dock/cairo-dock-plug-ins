/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include <stdlib.h>
#define __USE_POSIX
#include <signal.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

#define CD_CLOCK_TIMEZONE_DIR "/usr/share/zoneinfo"
static GList *s_pTimeZoneList = NULL;

void cd_clock_free_timezone_list (void)
{
	g_print ("%s ()\n", __func__);
	gpointer *data;
	GList *e;
	for (e = s_pTimeZoneList; e != NULL; e = e->next)
	{
		data = e->data;
		g_free (data[1]);
		g_free (data);
	}
	g_list_free (s_pTimeZoneList);
	s_pTimeZoneList = NULL;
}

static void _cd_clock_select_location (GtkMenuItem *pMenuItem, gpointer *data)
{
	CairoDockModuleInstance *myApplet = data[0];
	gchar *cLocationPath = data[1];
	g_print ("%s (%s, %s)\n", __func__, cLocationPath, myApplet->cConfFilePath);
	
	cairo_dock_update_conf_file (myApplet->cConfFilePath,
		G_TYPE_STRING, "Module", "location", cLocationPath,
		G_TYPE_INVALID);
	
	g_free (myConfig.cLocation);
	myConfig.cLocation = cLocationPath;
	data[1] = NULL;  // astuce.
	
	CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.cLocation+1);
	
	cd_clock_update_with_time (myApplet);
	
	cd_clock_free_timezone_list ();
}

static void _cd_clock_delete_menu (GtkMenuShell *menu, gpointer data)
{
	g_print ("%s ()\n", __func__);
}

int _cd_clock_compare_path_order (gpointer *data2, gpointer *data1) {
  gchar *cPath1 = data1[1], *cPath2 = data2[1];
	if (cPath1 == NULL)
		return -1;
	if (cPath2 == NULL)
		return 1;
	gchar *cURI_1 = g_ascii_strdown (cPath1, -1);
	gchar *cURI_2 = g_ascii_strdown (cPath2, -1);
	int iOrder = strcmp (cURI_1, cURI_2);
	g_free (cURI_1);
	g_free (cURI_2);
	return iOrder;
}

static GList *_parse_dir (const gchar *cDirPath, const gchar *cCurrentLocation, GtkWidget *pMenu, GList *pLocationPathList, CairoDockModuleInstance *myApplet)
{
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("clock : %s", erreur->message);
		g_error_free (erreur);
		return pLocationPathList;
	}
	
	GList *pPathList = pLocationPathList;
	GtkWidget *pMenuItem, *pSubMenu;
	const gchar *cFileName;
	GString *sFilePath = g_string_new ("");
	gchar *cLocationPath;
	gpointer *data;
	while ((cFileName = g_dir_read_name (dir)) != NULL)
	{
		if (g_str_has_suffix (cFileName, ".tab") || strcmp (cFileName, "posix") == 0 || strcmp (cFileName, "right") == 0)
			continue;
		
		pMenuItem = gtk_menu_item_new_with_label (cFileName);
		gtk_menu_shell_append (GTK_MENU_SHELL (pMenu), pMenuItem);
		
		if (cCurrentLocation != NULL)
			cLocationPath = g_strdup_printf ("%s/%s", cCurrentLocation, cFileName);
		else
			cLocationPath = g_strdup_printf (":%s", cFileName);
		
		g_string_printf (sFilePath, "%s/%s", cDirPath, cFileName);
		if (g_file_test (sFilePath->str, G_FILE_TEST_IS_DIR))
		{
			pSubMenu = gtk_menu_new ();
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);
			pPathList = _parse_dir (sFilePath->str, cLocationPath, pSubMenu, pPathList, myApplet);
			g_free (cLocationPath);
		}
		else
		{
			data = g_new (gpointer, 2);
			data[0] = myApplet;
			data[1] = cLocationPath;
			pPathList = g_list_prepend (pPathList, data);
			//pPathList = g_list_insert_sorted (pPathList, data, (GCompareFunc) _cd_clock_compare_path_order); //Sorted location is better no?
			g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK(_cd_clock_select_location), data);
		}
	}
	
	g_string_free (sFilePath, TRUE);
	g_dir_close (dir);
	return pPathList;
}
static void _cd_clock_search_for_location (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	GtkWidget *pMenu = gtk_menu_new ();
	if (s_pTimeZoneList != NULL)
		cd_clock_free_timezone_list ();
	s_pTimeZoneList = _parse_dir (CD_CLOCK_TIMEZONE_DIR, NULL, pMenu, NULL, myApplet);
	gtk_widget_show_all (pMenu);
	
	g_signal_connect_after (G_OBJECT (pMenu),
		"deactivate",
		G_CALLBACK (_cd_clock_delete_menu),
		NULL);
	
	gtk_menu_popup (GTK_MENU (pMenu),
		NULL,
		NULL,
		NULL,
		NULL,
		1,
		gtk_get_current_event_time ());
}

static void _cd_clock_launch_time_admin (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	GError *erreur = NULL;
	if (myConfig.cSetupTimeCommand != NULL)
	{
		g_spawn_command_line_async (myConfig.cSetupTimeCommand, &erreur);
	}
	else
	{
		if (! cairo_dock_fm_setup_time ())
		{
			if (g_iDesktopEnv == CAIRO_DOCK_KDE)
			{
				g_spawn_command_line_async ("kcmshell kde-clock.desktop", &erreur);
			}
			else
			{
				cd_warning ("couldn't guess what to do to set up time.");
			}
		}
	}
	if (erreur != NULL)
	{
		cd_warning ("Attention : when trying to execute '%s' : %s", myConfig.cSetupTimeCommand, erreur->message);
		g_error_free (erreur);
	}
}


CD_APPLET_ABOUT (D_("This is the Cairo-Dock's clock applet\n made by Fabrice Rey (fabounet@users.berlios.de) for Cairo-Dock.\nThe analogic representation is a port of the well-known Cairo-Clock\n from MacSlow (http://macslow.thepimp.net)."))


CD_APPLET_ON_CLICK_BEGIN
	if (myData.pCalendarDialog != NULL)
	{
		cairo_dock_dialog_unreference (myData.pCalendarDialog);
		myData.pCalendarDialog = NULL;
	}
	else
	{
		GtkWidget *pCalendar = gtk_calendar_new ();
		gchar *cImagePath= g_strconcat (MY_APPLET_SHARE_DATA_DIR, "/dates.svg", NULL);
		myData.pCalendarDialog = cairo_dock_show_dialog_full (_("Calendar"),
			myIcon, myContainer,
			0,
			cImagePath, /*GTK_BUTTONS_NONE,*/
			pCalendar,
			NULL, NULL, NULL);
		g_free (cImagePath);
	}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Clock", pSubMenu, CD_APPLET_MY_MENU);
		CD_APPLET_ADD_IN_MENU (D_("Set up time and date"), _cd_clock_launch_time_admin, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Choose a location"), _cd_clock_search_for_location, pSubMenu);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.iAlarmPID > 0)
	{
		kill (myData.iAlarmPID, 1);
		myData.iAlarmPID = 0;
	}
	cairo_dock_remove_dialog_if_any (myIcon);
	myData.pCalendarDialog = NULL;
CD_APPLET_ON_MIDDLE_CLICK_END
