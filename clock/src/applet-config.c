/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-config.h"

CD_APPLET_INCLUDE_MY_VARS


#define CD_CLOCK_MAX_NB_AlarmS 3
#define CD_CLOCK_NB_FREQUENCIES 12


CD_APPLET_GET_CONFIG_BEGIN
	//\_______________ On recupere les parametres de fonctionnement.
	myConfig.iShowDate 		= CD_CONFIG_GET_INTEGER ("Module", "show date");
	myConfig.bShowSeconds 		= CD_CONFIG_GET_BOOLEAN ("Module", "show seconds");
	myConfig.b24Mode 		= CD_CONFIG_GET_BOOLEAN ("Module", "24h mode");
	myConfig.bOldStyle 		= CD_CONFIG_GET_BOOLEAN ("Module", "old fashion style");
	double couleur[4] = {0., 0., 0.5, 1.};
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Module", "text color", myConfig.fTextColor, couleur);
	myConfig.cSetupTimeCommand 	= CD_CONFIG_GET_STRING ("Module", "setup command");
	myConfig.cFont = CD_CONFIG_GET_STRING ("Module", "font");
	if (myConfig.cFont == NULL)
		myConfig.cFont = g_strdup (g_iconTextDescription.cFont);
	
	myConfig.cLocation = CD_CONFIG_GET_STRING ("Module", "location");
	
	//\_______________ On recupere les alarmes.
	myConfig.pAlarms = g_ptr_array_new ();
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
	}
	g_string_free (sKeyName, TRUE);
	
	
	//\_______________ On liste les themes disponibles et on recupere celui choisi.
	myConfig.cThemePath = CD_CONFIG_GET_THEME_PATH ("Module", "theme", "themes", "default");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cThemePath);
	g_free (myConfig.cFont);
	g_free (myConfig.cLocation);

	CDClockAlarm *pAlarm;
	int i;
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
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	int i;
	for (i = 0; i < CLOCK_ELEMENTS; i ++)
	{
		if (myData.pSvgHandles[i] != NULL)
		{
			rsvg_handle_free (myData.pSvgHandles[i]);
		}
	}
	
	if (myData.pForegroundSurface != NULL)
	{
		cairo_surface_destroy (myData.pForegroundSurface);
	}
	if (myData.pBackgroundSurface != NULL)
	{
		cairo_surface_destroy (myData.pBackgroundSurface);
	}
	
	g_free (myData.cSystemLocation);
CD_APPLET_RESET_DATA_END
