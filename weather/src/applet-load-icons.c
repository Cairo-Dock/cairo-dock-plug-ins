/************************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

************************************************************************************/
#include <string.h>
#include <glib/gstdio.h>
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
	if (myData.days[i].cName != NULL)\
	{\
		pIcon = g_new0 (Icon, 1);\
		pIcon->acName = g_strdup_printf ("%s", myData.days[i].cName);\
		pIcon->acFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.days[i].part[j].cIconNumber);\
		if (! g_file_test (pIcon->acFileName, G_FILE_TEST_EXISTS))\
		{\
			g_free (pIcon->acFileName);\
			pIcon->acFileName = g_strdup_printf ("%s/%s.svg", myConfig.cThemePath, myData.days[i].part[j].cIconNumber);\
		}\
		if (myConfig.bDisplayTemperature)\
			pIcon->cQuickInfo = g_strdup_printf ("%s/%s", _display (myData.days[i].cTempMin), _display (myData.days[i].cTempMax));\
		pIcon->fOrder = 2*i+j;\
		pIcon->fScale = 1.;\
		pIcon->fAlpha = 1.;\
		pIcon->fWidthFactor = 1.;\
		pIcon->fHeightFactor = 1.;\
		pIcon->acCommand = g_strdup ("none");\
		pIcon->cParentDockName = g_strdup (myIcon->acName);\
		cd_debug (" + %s (%s , %s)\n", pIcon->acName, myData.days[i].part[j].cWeatherDescription, pIcon->acFileName);\
		pIconList = g_list_append (pIconList, pIcon);\
	}

static GList * _list_icons (void)
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
			erreur = NULL;
			myData.bErrorRetrievingData = TRUE;
		}
		else
			myData.bErrorRetrievingData = FALSE;
		g_remove (cCurrentConditionsFilePath);
		g_free (cCurrentConditionsFilePath);
	}
	
	if (cForecastFilePath != NULL)
	{
		cd_weather_parse_data (cForecastFilePath, FALSE, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			myData.bErrorRetrievingData = TRUE;
		}
		else
			myData.bErrorRetrievingData = FALSE;
		g_remove (cForecastFilePath);
		g_free (cForecastFilePath);
	}
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread (bErrorRetrievingData : %d)", myData.bErrorRetrievingData);
	return NULL;
}


static void _weather_draw_current_conditions (void)
{
	g_return_if_fail (myDrawContext != NULL);
	if (myConfig.bCurrentConditions)
	{
		cd_message ("  chargement de l'icone meteo");
		if (myConfig.bDisplayTemperature && myData.currentConditions.cTemp != NULL)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%s%s", myData.currentConditions.cTemp, myData.units.cTemp)
		}
		else
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
		}
		
		g_free (myIcon->acFileName);
		if (myData.bErrorRetrievingData)
			myIcon->acFileName = g_strdup_printf ("%s/broken.png", MY_APPLET_SHARE_DATA_DIR);
		else
		{
			myIcon->acFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.currentConditions.cIconNumber);
			if (! g_file_test (myIcon->acFileName, G_FILE_TEST_EXISTS))
			{
				g_free (myIcon->acFileName);
				myIcon->acFileName = g_strdup_printf ("%s/%s.svg", myConfig.cThemePath, myData.currentConditions.cIconNumber);
			}
		}
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName)
	}
}

static gboolean _cd_weather_check_for_redraw (gpointer data)
{
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning)
	{
		s_iSidTimerRedraw = 0;
		if (myIcon == NULL)
		{
			g_print ("annulation du chargement de la meteo\n");
			return FALSE;
		}
		
		//\_______________________ On cree la liste des icones de prevision.
		GList *pIconList = _list_icons ();
		
		//\_______________________ On efface l'ancienne liste.
		if (myDesklet && myDesklet->icons != NULL)
		{
			g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myDesklet->icons);
			myDesklet->icons = NULL;
			//myData.iNbIcons = 0;
			//myData.iMaxIconWidth = 0;
			//myDesklet->icons = NULL;
		}
		if (myIcon->pSubDock != NULL)
		{
			g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;
		}
		
		//\_______________________ On charge la nouvelle liste.
		if (myDock)  // en mode 'dock', on affiche la meteo dans un sous-dock.
		{
			if (myIcon->pSubDock == NULL)
			{
				if (pIconList != NULL)  // l'applet peut montrer les conditions courantes.
				{
					cd_message ("  creation du sous-dock meteo");
					myIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconList, myIcon->acName);
					cairo_dock_set_renderer (myIcon->pSubDock, myConfig.cRenderer);
					cairo_dock_update_dock_size (myIcon->pSubDock);
				}
			}
			else  // on a deja notre sous-dock, on remplace juste ses icones.
			{
				cd_message ("  rechargement du sous-dock meteo");
				if (pIconList == NULL)  // inutile de le garder.
				{
					cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
					myIcon->pSubDock = NULL;
				}
				else
				{
					myIcon->pSubDock->icons = pIconList;
					cairo_dock_load_buffers_in_one_dock (myIcon->pSubDock);
					cairo_dock_update_dock_size (myIcon->pSubDock);
				}
			}
		}
		else
		{
			if (myIcon->pSubDock != NULL)
			{
				cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
				myIcon->pSubDock = NULL;
			}
			myDesklet->icons = pIconList;
			gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
			cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, pConfig);
			myDrawContext = cairo_create (myIcon->pIconBuffer);
			gtk_widget_queue_draw (myDesklet->pWidget);
		}
		
		//\_______________________ On recharge l'icone principale.
		_weather_draw_current_conditions ();  // ne lance pas le redraw.
		if (myDesklet)
			gtk_widget_queue_draw (myDesklet->pWidget);
		else
			CD_APPLET_REDRAW_MY_ICON
		
		//\_______________________ On lance le timer si necessaire.
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
			s_iSidTimerRedraw = g_timeout_add (250, (GSourceFunc) _cd_weather_check_for_redraw, (gpointer) NULL);
		
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
