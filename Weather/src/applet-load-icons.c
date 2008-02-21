/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-read-data.h"
#include "applet-load-icons.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;


#define _add_icon(i, j)\
	pIcon = g_new0 (Icon, 1);\
	pIcon->acName = g_strdup_printf ("%s", myData.days[i].cName);\
	pIcon->acFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.days[i].part[j].cIconNumber);\
	if (myConfig.bDisplayTemperature)\
		pIcon->cQuickInfo = g_strdup_printf ("%s/%s", myData.days[i].cTempMin, myData.days[i].cTempMax);\
	pIcon->fOrder = 2*i+j;\
	pIcon->acCommand = g_strdup ("none");\
	pIcon->cParentDockName = g_strdup (myIcon->acName);\
	g_print (" + %s (%s)\n", pIcon->acName, myData.days[i].part[j].cWeatherDescription);\
	pIconList = g_list_append (pIconList, pIcon);

static GList * _load_icons (void)
{
	GList *pIconList = NULL;
	
	Icon *pIcon;
	int i;
	for (i = 0; i < myConfig.iNbDays; i ++)
	{
		_add_icon (i, 0);
		
		if (myConfig.bDisplayNights)
		{
			_add_icon (i, 1);
		}
	}
	
	return pIconList;
}


gboolean cd_weather_timer (gpointer data)
{
	cd_weather_launch_measure ();
	return TRUE;
}

gpointer cd_weather_threaded_calculation (gpointer data)
{
	GError *erreur = NULL;
	gchar *cCurrentConditionsFilePath = NULL, *cForecastFilePath = NULL;
	cd_weather_get_data (&cCurrentConditionsFilePath, &cForecastFilePath);
	
	if (cCurrentConditionsFilePath != NULL)
	{
		cd_weather_parse_data (cCurrentConditionsFilePath, TRUE, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
	
	if (cForecastFilePath != NULL)
	{
		cd_weather_parse_data (cForecastFilePath, FALSE, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread");
	return NULL;
}


static gboolean _cd_weather_check_for_redraw (gpointer data)
{
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning)
	{
		s_iSidTimerRedraw = 0;
		
		cd_message ("  chargement de l'icone meteo");
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%s%s", myData.currentConditions.cTemp, myData.units.cTemp)
		g_free (myIcon->acFileName);
		myIcon->acFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.currentConditions.cIconNumber);
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName)
		
		
		if (myIcon->pSubDock == NULL)
		{
			cd_message ("  creation du sous-dock meteo");
			GList *pIconList = _load_icons ();
			if (pIconList != NULL)  // l'applet peut montrer les conditions courantes.
			{
				myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
				cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
				cairo_dock_update_dock_size (myIcon->pSubDock);
			}
		}
		else
		{
			cd_message ("  rechargement du sous-dock meteo");
			cairo_dock_load_buffers_in_one_dock (myIcon->pSubDock);
		}
		
		if (myData.iSidTimer == 0)
			myData.iSidTimer = g_timeout_add (myConfig.iCheckInterval, (GSourceFunc) cd_weather_timer, NULL);
		return FALSE;
	}
	return TRUE;
}
void cd_weather_launch_measure (void)
{
	cd_message ("");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1))  // il etait egal a 0, on lui met 1 et on lance le thread.
	{
		cd_message (" ==> lancement du thread de calcul");
		
		if (s_iSidTimerRedraw == 0)
			s_iSidTimerRedraw = g_timeout_add (200, (GSourceFunc) _cd_weather_check_for_redraw, (gpointer) NULL);
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_weather_threaded_calculation,
			NULL,
			FALSE,
			&erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
}
