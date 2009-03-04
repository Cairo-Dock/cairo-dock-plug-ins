/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
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
	myConfig.bDisplayTemperature = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "display temperature", FALSE);
	myConfig.cDialogDuration = 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "dialog duration", 5);
	myConfig.iCheckInterval = 60 * MAX (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "check interval", 15), 1);
	
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "basic");
	
	myConfig.bDesklet3D = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "3D desket", FALSE);
	
	myConfig.cRenderer = CD_CONFIG_GET_STRING ("Configuration", "renderer");
CD_APPLET_GET_CONFIG_END


static void _reset_units (Unit *pUnits)
{
	g_free (pUnits->cTemp);
	g_free (pUnits->cDistance);
	g_free (pUnits->cSpeed);
	g_free (pUnits->cPressure);
}

static void _reset_current_conditions (CurrentContitions *pCurrentContitions)
{
	g_free (pCurrentContitions->cSunRise);
	g_free (pCurrentContitions->cSunSet);
	g_free (pCurrentContitions->cDataAcquisitionDate);
	g_free (pCurrentContitions->cObservatory);
	g_free (pCurrentContitions->cTemp);
	g_free (pCurrentContitions->cFeeledTemp);
	g_free (pCurrentContitions->cWeatherDescription);
	g_free (pCurrentContitions->cIconNumber);
	g_free (pCurrentContitions->cWindSpeed);
	g_free (pCurrentContitions->cWindDirection);
	g_free (pCurrentContitions->cPressure);
	g_free (pCurrentContitions->cHumidity);
	g_free (pCurrentContitions->cMoonIconNumber);
}

static void _reset_current_one_day (Day *pDay)
{
	g_free (pDay->cName);
	g_free (pDay->cDate);
	g_free (pDay->cTempMax);
	g_free (pDay->cTempMin);
	g_free (pDay->cSunRise);
	g_free (pDay->cSunSet);
	int j;
	for (j = 0; j < 2; j ++)
	{
		g_free (pDay->part[j].cIconNumber);
		g_free (pDay->part[j].cWeatherDescription);
		g_free (pDay->part[j].cWindSpeed);
		g_free (pDay->part[j].cWindDirection);
		g_free (pDay->part[j].cHumidity);
		g_free (pDay->part[j].cPrecipitationProba);
	}
}

CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cLocationCode);
	g_free (myConfig.cRenderer);
	g_free (myConfig.cThemePath);
CD_APPLET_RESET_CONFIG_END


void cd_weather_reset_all_datas (CairoDockModuleInstance *myApplet)
{
	cairo_dock_free_measure_timer (myData.pMeasureTimer);
	
	g_free (myData.cCCDataFilePath);
	g_free (myData.cForecastDataFilePath);
	g_free (myData.cLon);
	g_free (myData.cLat);
	_reset_units (&myData.units);
	_reset_current_conditions (&myData.currentConditions);
	int i;
	for (i = 0; i < myConfig.iNbDays; i ++)
	{
		_reset_current_one_day (&myData.days[i]);
	}
	
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
	GtkWidget *pCodeEntry = cairo_dock_get_widget_from_name ("Configuration", "location code");
	gtk_entry_set_text (GTK_ENTRY (pCodeEntry), cLocationCode);
	cd_weather_free_location_list ();
}
static void _cd_weather_search_for_location (GtkEntry *pEntry, CairoDockModuleInstance *myApplet)
{
	const gchar *cLocationName = gtk_entry_get_text (pEntry);
	if (cLocationName == NULL || *cLocationName == '\0')
		return;
	
	gchar *cFilePath = cd_weather_get_location_data (cLocationName);
	
	GError *erreur = NULL;
	cd_weather_free_location_list ();
	s_pLocationsList = cd_weather_parse_location_data (cFilePath, &erreur);
	if (erreur != NULL)
	{
		gchar *cIconPath = g_strdup_printf ("%s/broken.png", MY_APPLET_SHARE_DATA_DIR);
		cairo_dock_show_temporary_dialog_with_icon (D_("I couldn't get the info\n Is connexion alive ?"),
			myIcon,
			myContainer,
			0,
			cIconPath);
		g_free (cIconPath);
		
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
		
		g_error_free (erreur);
		erreur = NULL;
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
	
	g_remove (cFilePath);
	g_free (cFilePath);
}
void cd_weather_load_custom_widget (CairoDockModuleInstance *myApplet, GKeyFile* pKeyFile)
{
	g_print ("%s (%s)\n", __func__, myIcon->acName);
	//\____________ On recupere le widget.
	GtkWidget *pCodeEntry = cairo_dock_get_widget_from_name ("Configuration", "location code");
	g_return_if_fail (pCodeEntry != NULL);
	
	GtkWidget *pWidgetBox = gtk_widget_get_parent (pCodeEntry);
	
	GtkWidget *pLabel = gtk_label_new (D_("Search for your location :"));
	gtk_box_pack_start (GTK_BOX (pWidgetBox), pLabel, FALSE, FALSE, 0);
	
	GtkWidget *pLocationEntry = gtk_entry_new ();
	if (myData.cLocation != NULL)
		gtk_entry_set_text (GTK_ENTRY (pLocationEntry), myData.cLocation);
	gtk_box_pack_start (GTK_BOX (pWidgetBox), pLocationEntry, FALSE, FALSE, 0);
	g_signal_connect (pLocationEntry, "activate", G_CALLBACK (_cd_weather_search_for_location), myApplet);
}
