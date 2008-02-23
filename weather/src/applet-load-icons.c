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
	pIcon = g_new0 (Icon, 1);\
	pIcon->acName = g_strdup_printf ("%s", myData.days[i].cName);\
	pIcon->acFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.days[i].part[j].cIconNumber);\
	if (myConfig.bDisplayTemperature)\
		pIcon->cQuickInfo = g_strdup_printf ("%s/%s", myData.days[i].cTempMin, myData.days[i].cTempMax);\
	pIcon->fOrder = 2*i+j;\
	pIcon->fScale = 1.;\
	pIcon->fAlpha = 1.;\
	pIcon->fWidthFactor = 1.;\
	pIcon->fHeightFactor = 1.;\
	pIcon->acCommand = g_strdup ("none");\
	pIcon->cParentDockName = g_strdup (myIcon->acName);\
	cd_debug (" + %s (%s , %s)\n", pIcon->acName, myData.days[i].part[j].cWeatherDescription, pIcon->acFileName);\
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
		}
		g_remove (cForecastFilePath);
		g_free (cForecastFilePath);
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
		
		//\_______________________ On recharge l'icone principale.
		if (myConfig.bCurrentConditions)
		{
			cd_message ("  chargement de l'icone meteo");
			if (myConfig.bDisplayTemperature)
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("%s%s", myData.currentConditions.cTemp, myData.units.cTemp)
			}
			else
			{
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL)
			}
			g_free (myIcon->acFileName);
			myIcon->acFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.currentConditions.cIconNumber);
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName)
			if (myDock)
			{
				CD_APPLET_REDRAW_MY_ICON
			}
		}
		
		//\_______________________ On cree la liste des icones de prevision.
		GList *pIconList = _load_icons ();
		
		//\_______________________ On efface l'ancienne liste.
		if (myData.pDeskletIconList != NULL)
		{
			g_list_foreach (myData.pDeskletIconList, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myData.pDeskletIconList);
			myData.pDeskletIconList = NULL;
			myData.iMaxIconHeight = 0;
			myData.iMaxIconWidth = 0;
			myData.fLinearWidth = 0;
		}
		if (myIcon->pSubDock != NULL)
		{
			g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;
		}
		
		//\_______________________ On charge la nouvelle liste.
		if (myDock != NULL)  // en mode 'dock', on affiche la meteo dans un sous-dock.
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
				if (pIconList == NULL)  // inutile de la garder.
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
			myData.pDeskletIconList = pIconList;
			GList* ic;
			Icon *icon;
			cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
			for (ic = pIconList; ic != NULL; ic = ic->next)
			{
				icon = ic->data;
				if (myConfig.bDesklet3D)
				{
					icon->fWidth = 0;
					icon->fHeight = 0;
				}
				else
				{
					icon->fWidth = MAX (1, (myDesklet->iWidth - g_iDockRadius - myIcon->fWidth) / 2);
					icon->fHeight = MAX (1, (myDesklet->iHeight - g_iDockRadius - myIcon->fHeight) / 2);
				}
				cairo_dock_fill_icon_buffers (icon, pCairoContext, 1, CAIRO_DOCK_HORIZONTAL, myConfig.bDesklet3D);
				myData.iMaxIconHeight = MAX (myData.iMaxIconHeight, icon->fHeight);
				myData.iMaxIconWidth = MAX (myData.iMaxIconWidth, icon->fWidth);
				icon->fX = myData.fLinearWidth;
				myData.fLinearWidth += icon->fWidth;
			}
			cairo_destroy (pCairoContext);
			gtk_widget_queue_draw (myDesklet->pWidget);
		}
		myData.pFirstDrawnElement = myData.pDeskletIconList;
		
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
