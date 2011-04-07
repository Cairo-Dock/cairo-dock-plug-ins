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

#include <string.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-read-data.h"
#include "applet-config.h"

GList *s_pLocationsList = NULL;

CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.cLocationCode = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "location code", "FRXX0076");
	myConfig.bISUnits = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "IS units", TRUE);
	myConfig.bCurrentConditions = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display cc", TRUE);
	myConfig.bDisplayNights = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display nights", FALSE);
	myConfig.iNbDays = MIN (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb days", WEATHER_NB_DAYS_MAX), WEATHER_NB_DAYS_MAX);
	myConfig.bDisplayTemperature = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display temperature", TRUE);
	myConfig.cDialogDuration = 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "dialog duration", 7);
	myConfig.iCheckInterval = 60 * MAX (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 15), 1);
	
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "Classic");
	
	myConfig.bDesklet3D = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "3D desket", FALSE);
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
	
	gchar *cName = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.bSetName = (cName == NULL);
	g_free (cName);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cLocationCode);
	g_free (myConfig.cRenderer);
	g_free (myConfig.cThemePath);
CD_APPLET_RESET_CONFIG_END


void cd_weather_reset_all_datas (CairoDockModuleInstance *myApplet)
{
	cairo_dock_discard_task (myData.pTask);
	cairo_dock_discard_task (myData.pGetLocationTask);
	
	cd_weather_reset_data (myApplet);
	
	cd_weather_free_location_list ();
	
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	memset (myDataPtr, 0, sizeof (AppletData));
}

CD_APPLET_RESET_DATA_BEGIN
	cd_weather_reset_all_datas (myApplet);
CD_APPLET_RESET_DATA_END


  /////////////////////
 /// CUSTOM WIDGET ///
/////////////////////
void cd_weather_free_location_list (void)
{
	if (s_pLocationsList == NULL)
		return ;
	g_list_foreach (s_pLocationsList, (GFunc) g_free, NULL);
	g_list_free (s_pLocationsList);
	s_pLocationsList = NULL;
}

static void _cd_weather_location_choosed (GtkMenuItem *pMenuItem, gchar *cLocationCode)
{
	g_return_if_fail (cLocationCode != NULL);
	
	//\____________________ On met a jour le panneau de conf.
	CairoDockModuleInstance *myApplet = g_object_get_data (G_OBJECT (pMenuItem), "cd-applet");
	GtkWidget *pCodeEntry = CD_APPLET_GET_CONFIG_PANEL_WIDGET ("Configuration", "location code");
	if (pCodeEntry)
		gtk_entry_set_text (GTK_ENTRY (pCodeEntry), cLocationCode);
	cd_weather_free_location_list ();
}
static void _on_got_location_data (const gchar *cLocationData, CairoDockModuleInstance *myApplet)
{
	GError *erreur = NULL;
	cd_weather_free_location_list ();
	
	GtkWidget *pCodeEntry = CD_APPLET_GET_CONFIG_PANEL_WIDGET ("Configuration", "location code");
	if (!pCodeEntry)
	{
		g_print ("request took too long, discard results\n");
		cairo_dock_discard_task (myData.pGetLocationTask);
		myData.pGetLocationTask = NULL;
		return;
	}
	cairo_dock_set_status_message (NULL, "");
	
	s_pLocationsList = cd_weather_parse_location_data (cLocationData, &erreur);
	if (erreur != NULL)
	{
		gchar *cIconPath = g_strdup_printf ("%s/broken.png", MY_APPLET_SHARE_DATA_DIR);
		cairo_dock_show_temporary_dialog_with_icon (D_("I couldn't get the info\n Is connexion alive ?"),
			myIcon,
			myContainer,
			0,
			cIconPath);
		g_free (cIconPath);
		cairo_dock_set_status_message (NULL, D_("Couldn't get the location code (is connection alive?)"));
		
		g_error_free (erreur);
		erreur = NULL;  // on ne garde pas trace de l'erreur, c'est deja fait au (re)chargement.
	}
	else if (s_pLocationsList == NULL)
	{
		gchar *cIconPath = g_strdup_printf ("%s/broken.png", MY_APPLET_SHARE_DATA_DIR);
		cairo_dock_show_temporary_dialog_with_icon (D_("I couldn't find this location"),
			myIcon,
			myContainer,
			0,
			cIconPath);
		g_free (cIconPath);
		cairo_dock_set_status_message (NULL, "Location not available");
	}
	else
	{
		GtkWidget *pMenu = gtk_menu_new ();
		GString *sOneLocation = g_string_new ("");
		GtkWidget *pMenuItem;
		gchar *cLocationName, *cLocationCode;
		GList *list, *data;
		for (list = s_pLocationsList; list != NULL; list = list->next)
		{
			data = list;
			cLocationCode = list->data;
			list = list->next;
			cLocationName = list->data;
			
			g_string_printf (sOneLocation, "%s : %s", cLocationName, cLocationCode);
			pMenuItem = gtk_menu_item_new_with_label (sOneLocation->str);
			gtk_menu_shell_append  (GTK_MENU_SHELL (pMenu), pMenuItem);
			g_object_set_data (G_OBJECT (pMenuItem), "cd-applet", myApplet);
			g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK(_cd_weather_location_choosed), cLocationCode);
		}
		g_string_free (sOneLocation, TRUE);
		
		gtk_widget_show_all (pMenu);

		gtk_menu_popup (GTK_MENU (pMenu),
			NULL,
			NULL,
			NULL,
			NULL,
			1,
			gtk_get_current_event_time ());
	}
	
	cairo_dock_discard_task (myData.pGetLocationTask);
	myData.pGetLocationTask = NULL;
}
#define CD_WEATHER_BASE_URL "http://xml.weather.com"
static void _cd_weather_search_for_location (GtkEntry *pEntry, CairoDockModuleInstance *myApplet)
{
	const gchar *cLocationName = gtk_entry_get_text (pEntry);
	if (cLocationName == NULL || *cLocationName == '\0')
		return;
	
	cairo_dock_set_status_message_printf (NULL, D_("Searching the location code..."));
	
	if (myData.pGetLocationTask != NULL)
	{
		cairo_dock_discard_task (myData.pGetLocationTask);
		myData.pGetLocationTask = NULL;
	}
	gchar *cFileName = g_strdup_printf ("search?where=%s", cLocationName);
	myData.pGetLocationTask = cairo_dock_get_distant_file_content_async (CD_WEATHER_BASE_URL, "search", cFileName, (GFunc) _on_got_location_data, myApplet);
	g_free (cFileName);
}
void cd_weather_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile)
{
	cd_debug ("%s (%s)\n", __func__, myIcon->cName);
	//\____________ On recupere le widget.
	GtkWidget *pCodeEntry = CD_APPLET_GET_CONFIG_PANEL_WIDGET ("Configuration", "location code");
	g_return_if_fail (pCodeEntry != NULL);
	
	GtkWidget *pWidgetBox = gtk_widget_get_parent (pCodeEntry);
	
	GtkWidget *pLabel = gtk_label_new (D_("Search for your location :"));
	gtk_box_pack_start (GTK_BOX (pWidgetBox), pLabel, FALSE, FALSE, 0);
	
	GtkWidget *pLocationEntry = gtk_entry_new ();
	gtk_widget_set_tooltip_text (pLocationEntry, D_("Enter the name of your location and press Enter to choose amongst results."));
	if (myData.wdata.cLocation != NULL)
		gtk_entry_set_text (GTK_ENTRY (pLocationEntry), myData.wdata.cLocation);
	gtk_box_pack_start (GTK_BOX (pWidgetBox), pLocationEntry, FALSE, FALSE, 0);
	g_signal_connect (pLocationEntry, "activate", G_CALLBACK (_cd_weather_search_for_location), myApplet);
}
