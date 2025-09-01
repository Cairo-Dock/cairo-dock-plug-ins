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
#include <stdlib.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-theme.h"
#include "applet-calendar.h"
#include "applet-timer.h"
#include "applet-config.h"

#define CD_CLOCK_TIMEZONE_DIR "/usr/share/zoneinfo"
static GList *s_pTimeZoneList = NULL;

#define CD_CLOCK_NB_FREQUENCIES 12

// Translation Hack:
const char *strings_to_translate[] = {N_("Alarm"), N_("Time you want to be notified:"), N_("In the form xx:xx. E.g.: 20:35 for 8:35pm"), N_("Never"), N_("Day"), N_("Monday"), N_("Tuesday"), N_("Wednesday"), N_("Thursday"), N_("Friday"), N_("Saturday"), N_("Sunday"), N_("Week Day"), N_("Week End"), N_("Month"), N_("Repeat every:"), N_("If every month, which day of the month?"), N_("Message you want to use to be notified:"), N_("Tea Time!"), N_("Command to launch:"), N_("E.g.:")};

CD_APPLET_GET_CONFIG_BEGIN
	CD_CONFIG_RENAME_GROUP ("Module", "Configuration");
	
	myConfig.iShowDate 		= CD_CONFIG_GET_INTEGER ("Configuration", "show date");
	myConfig.bShowSeconds 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "show seconds");
	myConfig.iSmoothAnimationDuration = CD_CONFIG_GET_INTEGER_WITH_DEFAULT  ("Configuration", "smooth", 500);
	myConfig.b24Mode 			= CD_CONFIG_GET_BOOLEAN ("Configuration", "24h mode");
	myConfig.cLocation 		= CD_CONFIG_GET_STRING ("Configuration", "location");
	myConfig.cSetupTimeCommand 	= CD_CONFIG_GET_STRING ("Configuration", "setup command");
	myConfig.cTaskMgrName	 	= CD_CONFIG_GET_STRING ("Configuration", "task mgr");
	
	if (myConfig.iShowDate != CAIRO_DOCK_INFO_ON_LABEL && myConfig.cLocation != NULL)
	{
		gchar *cName = CD_CONFIG_GET_STRING ("Icon", "name");
		myConfig.bSetName = (cName == NULL);
		g_free (cName);
	}
	
	//\_______________ On recupere les parametres d'apparence.
	int iStyle = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "style", -1);  // si cette cle n'existait pas, elle a ete rajoutee avec la valeur -1.
	if (iStyle == -1)
	{
		cd_debug ("*** pas de cle 'style'");
		myConfig.bOldStyle = CD_CONFIG_GET_BOOLEAN ("Configuration", "old fashion style");
		g_key_file_set_integer (pKeyFile, "Configuration", "style", myConfig.bOldStyle ? 0 : 1);
	}
	else
	{
		myConfig.bOldStyle = (iStyle == 0);
	}
	
	if (myConfig.bOldStyle)
	{
		myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "glassy");
		CD_CONFIG_GET_COLOR_RGB ("Configuration", "date color", myConfig.fDateColor);
	}
	else
	{
		myConfig.bUseDefaultColors = (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "numeric style", 1) == 0);
		gboolean bCustomFont = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "custom font", FALSE);  // false by default
		if (! myConfig.bUseDefaultColors && bCustomFont)  // custom font
		{
			gchar *cFont = CD_CONFIG_GET_STRING ("Configuration", "font");
			gldi_text_description_set_font (&myConfig.textDescription, cFont);
		}
		else  // use the same font as the labels
		{
			gldi_text_description_copy (&myConfig.textDescription, &myStyleParam.textDescription);
		}
		pango_font_description_set_weight (myConfig.textDescription.fd, PANGO_WEIGHT_HEAVY);
		myConfig.textDescription.bNoDecorations = TRUE;
		
		if (! myConfig.bUseDefaultColors)  // custom colors
		{
			CD_CONFIG_GET_COLOR ("Configuration", "text color", &myConfig.textDescription.fColorStart);
			CD_CONFIG_GET_COLOR_RGBA ("Configuration", "outline color", myConfig.fOutlineColor);
			myConfig.iOutlineWidth = CD_CONFIG_GET_INTEGER ("Configuration", "outline width");
			myConfig.textDescription.bUseDefaultColors = FALSE;
		}
		else  // else, no outline and keep default colors
			myConfig.textDescription.bUseDefaultColors = TRUE;
		
		myConfig.cNumericBackgroundImage = CD_CONFIG_GET_STRING ("Configuration", "numeric bg");
		myConfig.fTextRatio = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "text ratio", 1.);
		
		myConfig.iPreferedTextLayout = CD_CONFIG_GET_INTEGER ("Configuration", "text layout");
	}
	
	//\_______________ On recupere les alarmes.
	myConfig.pAlarms = g_ptr_array_new ();
	CDClockAlarm *pAlarm;
	gboolean bAlarmOK;
	int iAlarmNumber=0, iHour, iMinute;
	GString *sKeyName = g_string_new ("");
	do
	{
		iAlarmNumber ++;
		bAlarmOK = FALSE;
		g_string_printf (sKeyName, "time%d", iAlarmNumber);
		if (! g_key_file_has_key (pKeyFile, "Alarm", sKeyName->str, NULL))
			break ;
		gchar *cUserTime = CD_CONFIG_GET_STRING ("Alarm", sKeyName->str);
		if (cUserTime != NULL)
		{
			if (sscanf(cUserTime, "%d:%d", &iHour, &iMinute) == 2 && iHour < 24 && iMinute < 59 && iHour >= 0 && iMinute >= 0)
				bAlarmOK = TRUE;
		}
		
		if (bAlarmOK)
		{
			pAlarm = g_new0 (CDClockAlarm, 1);
			g_ptr_array_add (myConfig.pAlarms, pAlarm);
			
			pAlarm->iHour = iHour;
			pAlarm->iMinute= iMinute;
			
			g_string_printf (sKeyName, "repeat%d", iAlarmNumber);
			int iFrequency = CD_CONFIG_GET_INTEGER ("Alarm", sKeyName->str);  // 1
			
			if (iFrequency > 0)
			{
				//g_print ("cette alarme a la frequence %d\n", iFrequency);
				if (iFrequency < 11)
					pAlarm->iDayOfWeek = iFrequency - 1;
				else
				{
					g_string_printf (sKeyName, "day%d", iAlarmNumber);
					pAlarm->iDayOfMonth = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Alarm", sKeyName->str, 1);
				}
			}
			
			g_string_printf (sKeyName, "message%d", iAlarmNumber);
			pAlarm->cMessage = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Alarm", sKeyName->str, "Wake Up !");
			g_string_printf (sKeyName, "command%d", iAlarmNumber);
			pAlarm->cCommand = CD_CONFIG_GET_STRING ("Alarm", sKeyName->str);
		}
	} while (1);
	g_string_free (sKeyName, TRUE);
	
	//\_______________ On recupere les parametres des taches.
	myConfig.bNormalDate = myConfig.b24Mode;
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cThemePath);
	gldi_text_description_reset (&myConfig.textDescription);
	g_free (myConfig.cLocation);
	g_free (myConfig.cDigital);
	g_free (myConfig.cNumericBackgroundImage);
	
	CDClockAlarm *pAlarm;
	guint i;
	if (myConfig.pAlarms != NULL)
	{
		for (i = 0; i < myConfig.pAlarms->len; i ++)
		{
			pAlarm = g_ptr_array_index (myConfig.pAlarms, i);
			cd_clock_free_alarm (pAlarm);
		}
		g_ptr_array_free (myConfig.pAlarms, TRUE);
	}
	
	g_free (myConfig.cSetupTimeCommand);
	
	g_free (myConfig.cTaskMgrName);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cd_clock_clear_theme (myApplet, TRUE);  // TRUE <=> tout.
	
	int i;
	for (i = 0; i < 4; i ++) {
		if (myData.pDigitalClock.pFrame[i].pFrameSurface != NULL)
			cairo_surface_destroy (myData.pDigitalClock.pFrame[i].pFrameSurface);
		
		if (myData.pDigitalClock.pText[i].pTextSurface != NULL)
			cairo_surface_destroy (myData.pDigitalClock.pText[i].pTextSurface);
	}
	
	g_free (myData.cSystemLocation);
	
	if (myData.pCalendarDialog != NULL)
		gldi_object_unref (GLDI_OBJECT(myData.pCalendarDialog));
	if (myData.pTaskWindow != NULL)
		gtk_widget_destroy (myData.pTaskWindow);
	
	cd_clock_reset_tasks_list (myApplet);
	g_hash_table_destroy (myData.pBackends);
	
	if (myData.tz) g_time_zone_unref (myData.tz);
CD_APPLET_RESET_DATA_END


  ///////////////
 /// ALARMES ///
///////////////
#define _add_new_entry(cEntryName, cEntryType, cDescription, cValue) do {\
	g_string_printf (sKeyName, cEntryName"%d", iNumNewAlarm);\
	g_key_file_set_string (pKeyFile, cGroupName, sKeyName->str, cValue);\
	g_key_file_set_comment (pKeyFile, cGroupName, sKeyName->str, cEntryType" "cDescription, NULL); } while (0)
#define _remove_entry(cEntryName) do {\
	g_string_printf (sKeyName, cEntryName"%d", iNumLastAlarm);\
	g_key_file_remove_comment (pKeyFile, cGroupName, sKeyName->str, NULL);\
	g_key_file_remove_key (pKeyFile, cGroupName, sKeyName->str, NULL); } while (0)
static void _cd_clock_add_alarm (GtkButton *button, GldiModuleInstance *myApplet)
{
	cd_debug ("%s (%d)", __func__, myConfig.pAlarms->len);
	//\____________ On ouvre notre fichier de conf.
	GError *erreur = NULL;
	GKeyFile* pKeyFile = g_key_file_new();
	g_key_file_load_from_file (pKeyFile, myApplet->cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("while trying to load %s : %s", myApplet->cConfFilePath, erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	//\____________ On recupere le numero de la derniere alarme.
	const gchar *cGroupName = "Alarm";
	int iNumNewAlarm = 0;
	GString *sKeyName = g_string_new ("");
	do
	{
		g_string_printf (sKeyName, "time%d", iNumNewAlarm+1);
		if (! g_key_file_has_key (pKeyFile, cGroupName, sKeyName->str, NULL))
			break ;
		iNumNewAlarm ++;
	} while (1);
	cd_debug ("%d alarmes deja presentes", iNumNewAlarm);
	iNumNewAlarm ++;
	
	//\____________ On rajoute les champs de la nouvelle alarme.
	_add_new_entry ("frame", "F0[Alarm]", "", "");
	_add_new_entry ("time", "s0", "Time you want to be notified:\n{In the form xx:xx. E.g.: 20:35 for 8:35pm}", "");
	_add_new_entry ("repeat", "l0[Never;Day;Monday;Tuesday;Wednesday;Thursday;Friday;Saturday;Sunday;Week Day;Week End;Month]", "Repeat every:", "0");
	_add_new_entry ("day", "i0[1;31]", "If every month, which day of the month?", "1");
	_add_new_entry ("message", "s0", "Message you want to use to be notified:", "Tea Time!");
	_add_new_entry ("command", "s0", "Command to launch:\n{E.g.: play /path/to/file.ogg}", "");
	
	cairo_dock_write_keys_to_file (pKeyFile, myApplet->cConfFilePath);
	g_key_file_free (pKeyFile);
	
	//\____________ On recharge le panneau de config.
	CD_APPLET_RELOAD_CONFIG_PANEL;
}
static void _cd_clock_remove_alarm (GtkButton *button, GldiModuleInstance *myApplet)
{
	cd_debug ("%s (%d)", __func__, myConfig.pAlarms->len);
	
	//\____________ On ouvre notre fichier de conf.
	GError *erreur = NULL;
	GKeyFile* pKeyFile = g_key_file_new();
	g_key_file_load_from_file (pKeyFile, myApplet->cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("while trying to load %s : %s", myApplet->cConfFilePath, erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	//\____________ On recupere le numero de la derniere alarme.
	const gchar *cGroupName = "Alarm";
	int iNumLastAlarm = 0;
	GString *sKeyName = g_string_new ("");
	do
	{
		g_string_printf (sKeyName, "time%d", iNumLastAlarm+1);
		if (! g_key_file_has_key (pKeyFile, cGroupName, sKeyName->str, NULL))
			break ;
		iNumLastAlarm ++;
	} while (1);
	cd_debug ("%d alarmes deja presentes", iNumLastAlarm);
	if (iNumLastAlarm == 0)
		return ;
	
	//\____________ On enleve les champs de la derniere alarme.
	_remove_entry ("frame");
	_remove_entry ("time");
	_remove_entry ("repeat");
	_remove_entry ("day");
	_remove_entry ("message");
	_remove_entry ("command");
	
	cairo_dock_write_keys_to_file (pKeyFile, myApplet->cConfFilePath);
	g_key_file_free (pKeyFile);
	
	//\____________ On recharge le panneau de config.
	CD_APPLET_RELOAD_CONFIG_PANEL;
}

  /////////////////
 /// LOCATIONS ///
/////////////////
static void _cd_clock_select_location (GtkMenuItem *pMenuItem, gpointer *data)
{
	GtkWidget *pLocationEntry = data[0];
	gchar *cLocationPath = data[1];
	g_return_if_fail (cLocationPath != NULL);
	
	//\____________________ On met a jour le panneau de conf.
	gtk_entry_set_text (GTK_ENTRY (pLocationEntry), cLocationPath);
	
	cd_clock_free_timezone_list ();
}

static void _cd_clock_delete_menu (GtkMenuShell *menu, gpointer data)
{
	cd_debug ("%s ()", __func__);
}

static int _cd_clock_compare_path_order (gpointer *data1, gpointer *data2) {
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
static GList *_cd_clock_parse_dir (const gchar *cDirPath, const gchar *cCurrentLocation, GtkWidget *pMenu, GList *pLocationPathList, GtkWidget *pLocationEntry)
{
	//\__________________ Ouverture du (sous-)repertoire.
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("clock : %s", erreur->message);
		g_error_free (erreur);
		return pLocationPathList;
	}
	
	//\__________________ On parcourt ce repertoire et on met tout dans une liste temporaire.
	GList *pPathList = pLocationPathList;
	GtkWidget *pMenuItem, *pSubMenu;
	const gchar *cFileName;
	GString *sFilePath = g_string_new ("");
	gchar *cLocationPath;
	gpointer *data;
	GList *pLocalList = NULL;  // liste temporaire ordonnee.
	gpointer *pLocalData;
	while ((cFileName = g_dir_read_name (dir)) != NULL)
	{
		// on saute les trucs inutiles.
		if (g_str_has_suffix (cFileName, ".tab") || strcmp (cFileName, "posix") == 0 || strcmp (cFileName, "right") == 0)
			continue;
		
		// on cree une nouvelle entree au menu, mais on ne la rajoute pas encore, on la stocke juste dans une liste temporaire, ordonnee par ordre alphabetique sur cFileName.
		pMenuItem = gtk_menu_item_new_with_label (cFileName);
		pLocalData = g_new (gpointer, 2);
		pLocalData[0] = pMenuItem;
		pLocalData[1] = (gpointer)cFileName;
		pLocalList = g_list_insert_sorted (pLocalList, pLocalData, (GCompareFunc) _cd_clock_compare_path_order);
		
		// location de cette entree.
		if (cCurrentLocation != NULL)
			cLocationPath = g_strdup_printf ("%s/%s", cCurrentLocation, cFileName);
		else
			cLocationPath = g_strdup_printf (":%s", cFileName);
		
		// si c'est un menu, on le parcourt et le menu item pointe sur le sous-menu, sinon il devient cliquable.
		g_string_printf (sFilePath, "%s/%s", cDirPath, cFileName);
		if (g_file_test (sFilePath->str, G_FILE_TEST_IS_DIR))
		{
			pSubMenu = gtk_menu_new ();  // inside the config window => use a normal GTK menu
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);
			pPathList = _cd_clock_parse_dir (sFilePath->str, cLocationPath, pSubMenu, pPathList, pLocationEntry);
			g_free (cLocationPath);
		}
		else
		{
			data = g_new (gpointer, 2);
			data[0] = pLocationEntry;
			data[1] = cLocationPath;
			pPathList = g_list_prepend (pPathList, data);
			g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK(_cd_clock_select_location), data);
		}
	}
	
	//\__________________ Maintenant qu'on a toutes les entrees de ce menu classes, on les insere dans l'ordre.
	GList *l;
	for (l = pLocalList; l != NULL; l = l->next)
	{
		pLocalData = l->data;
		gtk_menu_shell_append (GTK_MENU_SHELL (pMenu), pLocalData[0]);
		g_free (pLocalData);  // le filename appartient au GDir.
	}
	g_list_free (pLocalList);
	
	g_string_free (sFilePath, TRUE);
	g_dir_close (dir);
	return pPathList;
}
static void _cd_clock_search_for_location (GtkButton *pButton, GtkWidget *pLocationEntry)
{
	GtkWidget *pMenu = gtk_menu_new ();  // inside the config window => use a normal GTK menu
	if (s_pTimeZoneList != NULL)
		cd_clock_free_timezone_list ();
	s_pTimeZoneList = _cd_clock_parse_dir (CD_CLOCK_TIMEZONE_DIR, NULL, pMenu, NULL, pLocationEntry);
	gtk_widget_show_all (pMenu);
	
	g_signal_connect_after (G_OBJECT (pMenu),
		"deactivate",
		G_CALLBACK (_cd_clock_delete_menu),
		NULL);
	
	gtk_menu_popup_at_widget (GTK_MENU (pMenu),
		pLocationEntry,
		GDK_GRAVITY_SOUTH,
		GDK_GRAVITY_NORTH,
		NULL);
}

void cd_clock_free_timezone_list (void)
{
	cd_debug ("");
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

void cd_clock_load_custom_widget (GldiModuleInstance *myApplet, GKeyFile* pKeyFile, GSList *pWidgetList)  // warning: myApplet can be NULL if the applet has not been yet started.
{
	//\____________ On recupere notre widget personnalise (un simple container vide qu'on va remplir avec nos trucs).
	CairoDockGroupKeyWidget *pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Alarm", "add new");
	g_return_if_fail (pGroupKeyWidget != NULL);
	
	//\____________ On cree un bouton pour ajouter une alarme et on l'ajoute dans notre container.
	GtkWidget *pButton = gtk_button_new_from_icon_name (GLDI_ICON_NAME_ADD, GTK_ICON_SIZE_BUTTON);
	g_signal_connect (G_OBJECT (pButton),
		"clicked",
		G_CALLBACK (_cd_clock_add_alarm),
		myApplet);
	gtk_box_pack_end (GTK_BOX (pGroupKeyWidget->pKeyBox),
		pButton,
		FALSE,
		FALSE,
		0);
	//\____________ On cree un bouton pour supprimer une alarme et on l'ajoute dans notre container.
	pButton = gtk_button_new_from_icon_name (GLDI_ICON_NAME_REMOVE, GTK_ICON_SIZE_BUTTON);
	g_signal_connect (G_OBJECT (pButton),
		"clicked",
		G_CALLBACK (_cd_clock_remove_alarm),
		myApplet);
	gtk_box_pack_end (GTK_BOX (pGroupKeyWidget->pKeyBox),
		pButton,
		FALSE,
		FALSE,
		0);
	
	//\____________ On recupere le widget de la location (un gtk_entry).
	pGroupKeyWidget = cairo_dock_gui_find_group_key_widget_in_list (pWidgetList, "Configuration", "location");
	g_return_if_fail (pGroupKeyWidget != NULL);
	
	GtkWidget *pLocationEntry = cairo_dock_gui_get_first_widget (pGroupKeyWidget);
	g_return_if_fail (pLocationEntry != NULL);
	
	GtkWidget *pWidgetBox = gtk_widget_get_parent (pLocationEntry);
	
	GtkWidget *pLabel = gtk_label_new (D_("Search for your location :"));
	gtk_box_pack_start (GTK_BOX (pWidgetBox), pLabel, FALSE, FALSE, 0);
	
	GtkWidget *pLocationButton = gtk_button_new_from_icon_name (GLDI_ICON_NAME_FIND, GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (pWidgetBox), pLocationButton, FALSE, FALSE, 0);
	g_signal_connect (pLocationButton, "clicked", G_CALLBACK (_cd_clock_search_for_location), pLocationEntry);
}


void cd_clock_save_custom_widget (GldiModuleInstance *myApplet, GKeyFile *pKeyFile, GSList *pWidgetList)
{
	cd_debug ("%s (%s)", __func__, myIcon->cName);
	// ca c'est si on avait des valeurs a recuperer dans nos widgets personnalises, et a stocker dans le pKeyFile. mais ici ils sont simple, et donc tous pris en charge par le dock.
}
