/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <string.h>
#include <stdlib.h>

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-config.h"

extern CDClockDatePosition my_iShowDate;
extern gboolean my_bShowSeconds;
extern gboolean my_bOldStyle;
extern gboolean my_b24Mode;
extern double my_fTextColor[4];
//extern GHashTable *my_pThemeTable;
extern gchar *my_cThemePath;

extern RsvgDimensionData my_DimensionData;
extern RsvgHandle *my_pSvgHandles[CLOCK_ELEMENTS];
extern char my_cFileNames[CLOCK_ELEMENTS][30];
extern GPtrArray *my_pAlarms;

extern gchar *my_cSetupTimeCommand;

#define CD_CLOCK_MAX_NB_AlarmS 3
#define CD_CLOCK_NB_FREQUENCIES 12
static gchar *my_s_Frequencies[CD_CLOCK_NB_FREQUENCIES+1] = {"Never", "Day", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Week Day", "Week End", "Month", NULL};


CD_APPLET_CONFIG_BEGIN ("Horloge", NULL)
	//\_______________ On recupere les parametres de fonctionnement.
	my_iShowDate 		= CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Module", "show date", CLOCK_DATE_ON_LABEL);
	my_bShowSeconds 	= CD_CONFIG_GET_BOOLEAN ("Module", "show seconds");
	my_b24Mode 			= CD_CONFIG_GET_BOOLEAN ("Module", "24h mode");
	my_bOldStyle 			= CD_CONFIG_GET_BOOLEAN ("Module", "old fashion style");
	double couleur[4] = {0., 0., 0.5, 1.};
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Module", "text color", my_fTextColor, couleur);
	my_cSetupTimeCommand = CD_CONFIG_GET_STRING ("Module", "setup command");
	
	//\_______________ On recupere les alarmes.
	my_pAlarms = g_ptr_array_new ();
	CDClockAlarm *pAlarm;
	gboolean bAlarmOK;
	int iAlarmNumber, iHour, iMinute;
	GString *sKeyName = g_string_new ("");
	for (iAlarmNumber = 1; iAlarmNumber < CD_CLOCK_MAX_NB_AlarmS+1; iAlarmNumber ++)
	{
		bAlarmOK = FALSE;
		g_string_printf (sKeyName, "time%d", iAlarmNumber);
		gchar *cUserTime = CD_CONFIG_GET_STRING ("Alarm", sKeyName->str);
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
		}
	}
	g_string_free (sKeyName, TRUE);
	
	
	//\_______________ On liste les themes disponibles et on recupere celui choisi.
	my_cThemePath = CD_CONFIG_GET_THEME_PATH ("Module", "theme", "themes", "default");
	/*if (my_pThemeTable == NULL)
	{
		gchar *cThemesDir = g_strdup_printf ("%s/themes", MY_APPLET_SHARE_DATA_DIR);
		my_pThemeTable = cairo_dock_list_themes (cThemesDir, NULL, &erreur);
		if (erreur != NULL)
		{
			g_print ("Attention : %s\n", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		g_free (cThemesDir);
	}*/
CD_APPLET_CONFIG_END
