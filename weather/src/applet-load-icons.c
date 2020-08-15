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

#include "applet-struct.h"
#include "applet-read-data.h"
#include "applet-load-icons.h"

const char *cMonthsWeeks[19] = { N_("Monday") , N_("Tuesday") , N_("Wednesday") , N_("Thursday") , N_("Friday") , N_("Saturday") , N_("Sunday") , N_("Jan") , N_("Feb") , N_("Mar") , N_("Apr") , N_("May") ,N_("Jun") , N_("Jui") , N_("Aug") , N_("Sep") , N_("Oct") , N_("Nov") , N_("Dec") };  // pour qu'ils soient listes dans le .pot.

#define _add_icon(i, j)\
	if (myData.wdata.days[i].cName != NULL)\
	{\
		pIcon = cairo_dock_create_dummy_launcher (g_strdup ((gchar *)myData.wdata.days[i].cName),\
			g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.wdata.days[i].cIconNumber),\
			NULL,\
			(myConfig.bDisplayTemperature ? g_strdup_printf ("%s/%s", _display (myData.wdata.days[i].cTempMin), _display (myData.wdata.days[i].cTempMax)) : NULL),\
			2*i+j);\
		if (! g_file_test (pIcon->cFileName, G_FILE_TEST_EXISTS))\
		{\
			g_free (pIcon->cFileName);\
			pIcon->cFileName = g_strdup_printf ("%s/%s.svg", myConfig.cThemePath, myData.wdata.days[i].cIconNumber);\
		}\
		cairo_dock_listen_for_double_click (pIcon);\
		pIconList = g_list_append (pIconList, pIcon);\
	}

static GList * _list_icons (GldiModuleInstance *myApplet)
{
	GList *pIconList = NULL;
	
	Icon *pIcon;
	int i;
	for (i = 0; i < myConfig.iNbDays; i ++)
	{
		_add_icon (i, 0);
	}
	
	return pIconList;
}


static void _weather_draw_current_conditions (GldiModuleInstance *myApplet)
{
	if (myConfig.bCurrentConditions || myData.bErrorRetrievingData)
	{
		cd_message ("  chargement de l'icone meteo (%x)", myApplet);
		if (myConfig.bDisplayTemperature && myData.wdata.currentConditions.now.cTempMax != NULL)
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%s%s", myData.wdata.currentConditions.now.cTempMax, myData.wdata.units.cTemp);
		}
		else
		{
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		}
		
		g_free (myIcon->cFileName);
		if (myData.bErrorRetrievingData)
		{
			myIcon->cFileName = g_strdup_printf ("%s/na.png", myConfig.cThemePath);
			if (! g_file_test (myIcon->cFileName, G_FILE_TEST_EXISTS))
			{
				g_free (myIcon->cFileName);
				myIcon->cFileName = g_strdup_printf ("%s/na.svg", myConfig.cThemePath);
				if (! g_file_test (myIcon->cFileName, G_FILE_TEST_EXISTS))
				{
					g_free (myIcon->cFileName);
					myIcon->cFileName = g_strdup (MY_APPLET_SHARE_DATA_DIR"/broken.png");
				}
			}
		}
		else
		{
			myIcon->cFileName = g_strdup_printf ("%s/%s.png", myConfig.cThemePath, myData.wdata.currentConditions.now.cIconNumber);
			if (! g_file_test (myIcon->cFileName, G_FILE_TEST_EXISTS))
			{
				g_free (myIcon->cFileName);
				myIcon->cFileName = g_strdup_printf ("%s/%s.svg", myConfig.cThemePath, myData.wdata.currentConditions.now.cIconNumber);
			}
		}
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->cFileName);
	}
	else
	{
		CD_APPLET_SET_DEFAULT_IMAGE_ON_MY_ICON_IF_NONE;
	}
}



gboolean cd_weather_update_from_data (CDSharedMemory *pSharedMemory)
{
	GldiModuleInstance *myApplet = pSharedMemory->pApplet;
	g_return_val_if_fail (myIcon != NULL, FALSE);  // paranoia
	CD_APPLET_ENTER;
	
	if (myData.bBusy)
	{
		myData.bBusy = FALSE;
		CD_APPLET_STOP_ANIMATING_MY_ICON;
	}
	
	//\_______________________ in case an error occured, keep the current data, and just redraw the main icon.
	if (pSharedMemory->bErrorInThread)
	{
		if (!myData.bErrorRetrievingData)  // no previous error, draw an emblem.
		{
			myData.bErrorRetrievingData = TRUE;
			
			_weather_draw_current_conditions (myApplet);  // draw the icon, in case we never drawn the icon before.
			
			// retry in 20s, in case it's just a temporary network loss, or a slow connection on startup.
			if (myData.pTask->iPeriod > 20)
			{
				cd_message ("no data, will re-try in 20s");
				gldi_task_change_frequency (myData.pTask, 20);
			}
			
		}
		cd_weather_reset_weather_data (&pSharedMemory->wdata);  // discard the results, since they are probably empty or incomplete.
		memset (&pSharedMemory->wdata, 0, sizeof (CDWeatherData));
		
		CD_APPLET_LEAVE (TRUE);  // don't recreate the icons, since data have not changed.
	}
	myData.bErrorRetrievingData = FALSE;
	
	//\_______________________ copy the shared memory into our data.
	// free the current saved data.
	cd_weather_reset_data (myApplet);

	// then save the new data.
	memcpy (&myData.wdata, &pSharedMemory->wdata, sizeof (CDWeatherData));
	
	// and clear the shared memory.
	memset (&pSharedMemory->wdata, 0, sizeof (CDWeatherData));
	
	//\_______________________ On etablit le nom de l'icone.
	if ((myIcon->cName == NULL || myData.bSetName) && myDock)
	{
		myData.bSetName = (myData.wdata.cCity == NULL);  // cas ou l'applet demarre avant l'etablissesment de la connexion.
		gchar *cLocation = g_strdup_printf("%s, %s", myData.wdata.cCity, myData.wdata.cCountry);
		CD_APPLET_SET_NAME_FOR_MY_ICON (myData.wdata.cCity != NULL ? cLocation : WEATHER_DEFAULT_NAME);
		g_free (cLocation);
	}
	
	//\_______________________ On cree la liste des icones de prevision.
	GList *pIconList = _list_icons (myApplet);  // ne nous appartiendra plus, donc ne pas desallouer.
	
	//\_______________________ On efface l'ancienne liste.
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	//\_______________________ On charge la nouvelle liste.
	if (pIconList != NULL)
	{
		///gpointer pConfig[2] = {GINT_TO_POINTER (myConfig.bDesklet3D), GINT_TO_POINTER (FALSE)};
		///CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, "Caroussel", pConfig);
		gdouble white[4] = {1., 1., 1., .4};
		gpointer pConfig[3] = {GINT_TO_POINTER (1), GINT_TO_POINTER (FALSE), white};
		CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, "Panel", pConfig);
	}
	else if (myDock)  // sinon on ne veut pas du sous-dock vide.
	{
		gldi_object_unref (GLDI_OBJECT(myIcon->pSubDock));
		myIcon->pSubDock = NULL;
	}
	if (myDesklet)
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	
	//\_______________________ On recharge l'icone principale.
	_weather_draw_current_conditions (myApplet);  // ne lance pas le redraw.
	CD_APPLET_REDRAW_MY_ICON;
	
	if (myData.wdata.currentConditions.ttl <= 0)
		myData.wdata.currentConditions.ttl = 60;  // 1h by default
	cd_message ("next fetching in %dmn", myData.wdata.currentConditions.ttl);
	gldi_task_change_frequency (myData.pTask, myData.wdata.currentConditions.ttl * 60);
	
	CD_APPLET_LEAVE (TRUE);
}
