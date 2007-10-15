/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <string.h>
#include <stdlib.h>

#include <cairo-dock.h>

#include "cd-clock-struct.h"
#include "cd-clock-draw.h"
#include "cd-clock-config.h"

extern gboolean my_bShowDate;
extern gboolean my_bShowSeconds;
extern gboolean my_bOldStyle;
extern gboolean my_b24Mode;
extern double my_fTextColor[4];
extern GHashTable *my_pThemeTable;

extern RsvgDimensionData my_DimensionData;
extern RsvgHandle *my_pSvgHandles[CLOCK_ELEMENTS];
extern char my_cFileNames[CLOCK_ELEMENTS][30];
extern GPtrArray *my_pAlarms;

#define CD_CLOCK_MAX_NB_ALARMS 3
#define CD_CLOCK_NB_FREQUENCIES 12
static gchar *my_s_Frequencies[CD_CLOCK_NB_FREQUENCIES+1] = {"Never", "Day", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Week Day", "Week End", "Month", NULL};


static void _cd_clock_load_alarms (GKeyFile *pKeyFile, gboolean *bFlushConfFileNeeded)
{
	CDClockAlarm *pAlarm;
	gboolean bAlarmOK;
	int iAlarmNumber, iHour, iMinute;
	GString *sKeyName = g_string_new ("");
	
	for (iAlarmNumber = 1; iAlarmNumber < CD_CLOCK_MAX_NB_ALARMS+1; iAlarmNumber ++)
	{
		bAlarmOK = FALSE;
		g_string_printf (sKeyName, "time%d", iAlarmNumber);
		gchar *cUserTime = cairo_dock_get_string_key_value (pKeyFile, "ALARM", sKeyName->str, bFlushConfFileNeeded, NULL);
		if (cUserTime != NULL)
		{
			if (sscanf(cUserTime, "%d:%d", &iHour, &iMinute) == 2 && iHour < 24 && iMinute < 59 && iHour >= 0 && iMinute >= 0)
				bAlarmOK = TRUE;
		}
		
		if (bAlarmOK)
		{
			pAlarm = g_new0 (CDClockAlarm, 1);
			g_ptr_array_add (my_pAlarms, pAlarm);
			
			pAlarm->iHour = iHour;
			pAlarm->iMinute= iMinute;
			
			g_string_printf (sKeyName, "repeat%d", iAlarmNumber);
			gchar *cFrequency = cairo_dock_get_string_key_value (pKeyFile, "ALARM", sKeyName->str, bFlushConfFileNeeded, "Every Day");
			int iFrequency = cairo_dock_get_number_from_name (cFrequency, my_s_Frequencies);
			
			if (iFrequency > 0)
			{
				//g_print ("cette alarme a la frequence %d\n", iFrequency);
				if (iFrequency < 11)
					pAlarm->iDayOfWeek = iFrequency - 1;
				else
				{
					g_string_printf (sKeyName, "day%d", iAlarmNumber);
					pAlarm->iDayOfMonth = cairo_dock_get_integer_key_value (pKeyFile, "ALARM", sKeyName->str, bFlushConfFileNeeded, 1);
				}
			}
			
			g_string_printf (sKeyName, "message%d", iAlarmNumber);
			pAlarm->cMessage = cairo_dock_get_string_key_value (pKeyFile, "ALARM", sKeyName->str, bFlushConfFileNeeded, "Wake Up !");
		}
	}
	g_string_free (sKeyName, TRUE);
}


void cd_clock_read_conf_file (gchar *cConfFilePath, int *iWidth, int *iHeight, gchar **cName)
{
	GError *erreur = NULL;
	
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	GKeyFile *pKeyFile = cairo_dock_read_header_applet_conf_file (cConfFilePath, iWidth, iHeight, cName, &bFlushConfFileNeeded);
	g_return_if_fail (pKeyFile != NULL);
	
	my_bShowDate = cairo_dock_get_boolean_key_value (pKeyFile, "MODULE", "show date", &bFlushConfFileNeeded, TRUE);
	
	my_bShowSeconds = cairo_dock_get_boolean_key_value (pKeyFile, "MODULE", "show seconds", &bFlushConfFileNeeded, TRUE);
	
	my_b24Mode = cairo_dock_get_boolean_key_value (pKeyFile, "MODULE", "24h mode", &bFlushConfFileNeeded, TRUE);
	
	my_bOldStyle = cairo_dock_get_boolean_key_value (pKeyFile, "MODULE", "old fashion style", &bFlushConfFileNeeded, FALSE);
	
	gchar *cThemeName = cairo_dock_get_string_key_value (pKeyFile, "MODULE", "theme", &bFlushConfFileNeeded, "default");
	
	double couleur[4] = {0., 0., 0.5, 1.};
	cairo_dock_get_double_list_key_value (pKeyFile, "MODULE", "text color", &bFlushConfFileNeeded, my_fTextColor, 4, couleur);
	
	my_pAlarms = g_ptr_array_new ();
	_cd_clock_load_alarms (pKeyFile, &bFlushConfFileNeeded);
	
	
	//\_______________ On charge le theme choisi.
	if (cThemeName != NULL)
	{
		gchar *cThemePath = g_hash_table_lookup (my_pThemeTable, cThemeName);
		if (cThemePath == NULL)
			cThemePath = g_hash_table_lookup (my_pThemeTable, "default");
		g_return_if_fail (cThemePath != NULL);
		gchar *cElementPath;
		int i;
		for (i = 0; i < CLOCK_ELEMENTS; i ++)
		{
			cElementPath = g_strdup_printf ("%s/%s", cThemePath, my_cFileNames[i]);
			
			my_pSvgHandles[i] = rsvg_handle_new_from_file (cElementPath, NULL);
			//g_print (" + %s\n", cElementPath);
			g_free (cElementPath);
		}
		rsvg_handle_get_dimensions (my_pSvgHandles[CLOCK_DROP_SHADOW], &my_DimensionData);
	}
	else
	{
		my_DimensionData.width = 48;  // valeur par defaut si aucun theme.
		my_DimensionData.height = 48;
	}
	
	
	if (bFlushConfFileNeeded)
	{
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, CD_CLOCK_SHARE_DATA_DIR);
		/*gchar *cCommand = g_strdup_printf ("/bin/cp %s/%s %s", CD_CLOCK_SHARE_DATA_DIR, CD_CLOCK_CONF_FILE, cConfFilePath);
		system (cCommand);
		g_free (cCommand);
		
		cairo_dock_replace_values_in_conf_file (cConfFilePath, pKeyFile, TRUE, 0);*/
		
		cairo_dock_update_conf_file_with_hash_table (cConfFilePath, my_pThemeTable, "MODULE", "theme", NULL, (GHFunc) cairo_dock_write_one_theme_name);
	}	
	g_key_file_free (pKeyFile);
}
