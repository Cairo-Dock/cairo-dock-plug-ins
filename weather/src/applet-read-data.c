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

#define _BSD_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"

#define CD_WEATHER_BASE_URL "http://xml.weather.com"


static xmlDocPtr _cd_weather_open_xml_buffer (const gchar *cData, xmlNodePtr *root_node, const gchar *cRootNodeName, GError **erreur)
{
	if (cData == NULL || *cData == '\0')
	{
		g_set_error (erreur, 1, 1, "empty data (no connection ?)");
		return NULL;
	}
	int length = strlen (cData);
	
	gchar *cRootNode = g_strdup_printf ("<%s ", cRootNodeName);
	if (g_strstr_len (cData, length, cRootNode) == NULL)  // on intercepte le cas ou une connexion a un hotspot nous renvoie une page meme quand la connexion a weather.com n'a pas pu se faire, car ca fait planter libxml.
	{
		g_set_error (erreur, 1, 1, "uncorrect data (no connection ?)");
		g_free (cRootNode);
		return NULL;
	}
	g_free (cRootNode);
	xmlInitParser ();
	
	xmlDocPtr doc = xmlParseMemory (cData, length);
	if (doc == NULL)
	{
		g_set_error (erreur, 1, 1, "uncorrect data (no connection ?)");
		return NULL;
	}
	
	xmlNodePtr noeud = xmlDocGetRootElement (doc);
	if (noeud == NULL || xmlStrcmp (noeud->name, (const xmlChar *) cRootNodeName) != 0)
	{
		g_set_error (erreur, 1, 2, "xml data is not well formed (weather.com may have changed its data format)");
		return doc;
	}
	*root_node = noeud;
	return doc;
}
static xmlDocPtr _cd_weather_open_xml_file (const gchar *cDataFilePath, xmlNodePtr *root_node, const gchar *cRootNodeName, GError **erreur)
{
	gsize length = 0;
	gchar *cContent = NULL;
	g_file_get_contents (cDataFilePath,
		&cContent,
		&length,
		NULL);
	xmlDocPtr doc = _cd_weather_open_xml_buffer (cContent, root_node, cRootNodeName, erreur);
	g_free (cContent);
	return doc;
}
static void _cd_weather_close_xml_file (xmlDocPtr doc)
{
	if (doc != NULL)
		xmlFreeDoc (doc);  // pas de xmlCleanupParser, ca fout le boxon.
}


GList *cd_weather_parse_location_data (const gchar *cData, GError **erreur)
{
	GError *tmp_erreur = NULL;
	xmlNodePtr noeud = NULL;
	xmlDocPtr doc = _cd_weather_open_xml_buffer (cData, &noeud, "search", &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		_cd_weather_close_xml_file (doc);
		return NULL;
	}
	
	GList *cLocationsList = NULL;
	xmlNodePtr param;
	for (param = noeud->xmlChildrenNode; param != NULL; param = param->next)
	{
		if (xmlStrcmp (param->name, (const xmlChar *) "loc") == 0)
		{
			cLocationsList = g_list_prepend (cLocationsList, xmlNodeGetContent (param));
			cLocationsList = g_list_prepend (cLocationsList,  xmlGetProp (param, (xmlChar *) "id"));
		}
	}
	_cd_weather_close_xml_file (doc);
	return cLocationsList;
}


static void _cd_weather_parse_data (CDSharedMemory *pSharedMemory, const gchar *cData, gboolean bParseHeader, GError **erreur)
{
	GError *tmp_erreur = NULL;
	xmlNodePtr noeud = NULL;
	xmlDocPtr doc = _cd_weather_open_xml_buffer (cData, &noeud, "weather", &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		_cd_weather_close_xml_file (doc);
		return ;
	}
	
	xmlNodePtr param, fils, petitfils, arrpetitfils, arrarrpetitfils;
	gchar *nom, *visible, *name, *defaultsource = NULL, *source, *where;
	xmlChar *contenu;
	int i, j;
	gchar *index_str, *cDayName, *cDate, *str;
	for (param = noeud->xmlChildrenNode; param != NULL; param = param->next)
	{
		if (bParseHeader && xmlStrcmp (param->name, (const xmlChar *) "head") == 0)
		{
			for (fils = param->children; fils != NULL; fils = fils->next)
			{
				if (xmlStrcmp (fils->name, (const xmlChar *) "ut") == 0)
				{
					gchar *degree = xmlNodeGetContent (fils);
					if (degree == NULL || strncmp (degree, "°", strlen ("°")) != 0)  // prepend � if not present.
					{
						pSharedMemory->wdata.units.cTemp = g_strconcat ("°", degree, NULL);
						g_free (degree);
					}
					else
						pSharedMemory->wdata.units.cTemp = degree;
				}
				else if (xmlStrcmp (fils->name, (const xmlChar *) "ud") == 0)
					pSharedMemory->wdata.units.cDistance = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "us") == 0)
					pSharedMemory->wdata.units.cSpeed = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "up") == 0)
					pSharedMemory->wdata.units.cPressure = xmlNodeGetContent (fils);
				//else if (xmlStrcmp (fils->name, (const xmlChar *) "ur") == 0)  // ?
				//	pSharedMemory->wdata.units.cR = xmlNodeGetContent (fils);
			}
		}
		else if (bParseHeader && xmlStrcmp (param->name, (const xmlChar *) "loc") == 0)
		{
			for (fils = param->children; fils != NULL; fils = fils->next)
			{
				if (xmlStrcmp (fils->name, (const xmlChar *) "dnam") == 0)
					pSharedMemory->wdata.cLocation = xmlNodeGetContent (fils);
				/**else if (xmlStrcmp (fils->name, (const xmlChar *) "lat") == 0)
					pSharedMemory->cLat = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "lon") == 0)
					pSharedMemory->cLon = xmlNodeGetContent (fils);*/
				else if (xmlStrcmp (fils->name, (const xmlChar *) "sunr") == 0)
					pSharedMemory->wdata.currentConditions.cSunRise = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "suns") == 0)
					pSharedMemory->wdata.currentConditions.cSunSet = xmlNodeGetContent (fils);
			}
		}
		else if (xmlStrcmp (param->name, (const xmlChar *) "cc") == 0)
		{
			for (fils = param->children; fils != NULL; fils = fils->next)
			{
				if (xmlStrcmp (fils->name, (const xmlChar *) "lsup") == 0)
					pSharedMemory->wdata.currentConditions.cDataAcquisitionDate = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "obst") == 0)
					pSharedMemory->wdata.currentConditions.cObservatory = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "tmp") == 0)
					pSharedMemory->wdata.currentConditions.cTemp = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "flik") == 0)
					pSharedMemory->wdata.currentConditions.cFeltTemp = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "t") == 0)
					pSharedMemory->wdata.currentConditions.cWeatherDescription = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "icon") == 0)
					pSharedMemory->wdata.currentConditions.cIconNumber = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "wind") == 0)
				{
					for (petitfils = fils->children; petitfils != NULL; petitfils = petitfils->next)
					{
						if (xmlStrcmp (petitfils->name, (const xmlChar *) "s") == 0)
							pSharedMemory->wdata.currentConditions.cWindSpeed = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "t") == 0)
							pSharedMemory->wdata.currentConditions.cWindDirection = xmlNodeGetContent (petitfils);
					}
				}
				else if (xmlStrcmp (fils->name, (const xmlChar *) "bar") == 0)
				{
					for (petitfils = fils->children; petitfils != NULL; petitfils = petitfils->next)
					{
						if (xmlStrcmp (petitfils->name, (const xmlChar *) "r") == 0)
							pSharedMemory->wdata.currentConditions.cPressure = xmlNodeGetContent (petitfils);
					}
				}
				else if (xmlStrcmp (fils->name, (const xmlChar *) "hmid") == 0)
					pSharedMemory->wdata.currentConditions.cHumidity = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "moon") == 0)
				{
					for (petitfils = fils->children; petitfils != NULL; petitfils = petitfils->next)
					{
						if (xmlStrcmp (petitfils->name, (const xmlChar *) "icon") == 0)
							pSharedMemory->wdata.currentConditions.cMoonIconNumber = xmlNodeGetContent (petitfils);
					}
				}
			}
		}
		else if (xmlStrcmp (param->name, (const xmlChar *) "dayf") == 0)
		{
			for (fils = param->children; fils != NULL; fils = fils->next)
			{
				if (xmlStrcmp (fils->name, (const xmlChar *) "lsup") == 0)
					pSharedMemory->wdata.currentConditions.cDataAcquisitionDate = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "day") == 0)
				{
					index_str = (gchar *) xmlGetProp (fils, (xmlChar *) "d");
					if (index_str == NULL)
						continue;
					i = atoi (index_str);
					g_free (index_str);
					cDayName = (gchar *) xmlGetProp (fils, (xmlChar *) "t");
					pSharedMemory->wdata.days[i].cName = g_strdup (D_(cDayName));
					g_free (cDayName);
					cDate = (gchar *) xmlGetProp (fils, (xmlChar *) "dt");
					str = strchr (cDate, ' ');
					if (str != NULL)
					{
						*str = '\0';
						pSharedMemory->wdata.days[i].cDate = g_strconcat (D_(cDate), " ", str+1, NULL);
						g_free (cDate);
					}
					else
						pSharedMemory->wdata.days[i].cDate = cDate;
					for (petitfils = fils->children; petitfils != NULL; petitfils = petitfils->next)
					{
						if (xmlStrcmp (petitfils->name, (const xmlChar *) "hi") == 0)
							pSharedMemory->wdata.days[i].cTempMax = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "low") == 0)
							pSharedMemory->wdata.days[i].cTempMin = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "sunr") == 0)
							pSharedMemory->wdata.days[i].cSunRise = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "suns") == 0)
							pSharedMemory->wdata.days[i].cSunSet = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "part") == 0)
						{
							index_str = (gchar *) xmlGetProp (petitfils, (xmlChar *) "p");
							if (index_str == NULL)
								continue;
							j = (*index_str == 'd' ? 0 : 1);  // jour : 0 / nuit : 1.
							for (arrpetitfils = petitfils->children; arrpetitfils != NULL; arrpetitfils = arrpetitfils->next)
							{
								if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "icon") == 0)
									pSharedMemory->wdata.days[i].part[j].cIconNumber = xmlNodeGetContent (arrpetitfils);
								else if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "t") == 0)
									pSharedMemory->wdata.days[i].part[j].cWeatherDescription = xmlNodeGetContent (arrpetitfils);
								else if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "wind") == 0)
								{
									for (arrarrpetitfils = arrpetitfils->children; arrarrpetitfils != NULL; arrarrpetitfils = arrarrpetitfils->next)
									{
										if (xmlStrcmp (arrarrpetitfils->name, (const xmlChar *) "s") == 0)
											pSharedMemory->wdata.days[i].part[j].cWindSpeed = xmlNodeGetContent (arrarrpetitfils);
										else if (xmlStrcmp (arrarrpetitfils->name, (const xmlChar *) "t") == 0)
											pSharedMemory->wdata.days[i].part[j].cWindDirection = xmlNodeGetContent (arrarrpetitfils);
									}
								}
								else if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "hmid") == 0)
									pSharedMemory->wdata.days[i].part[j].cHumidity = xmlNodeGetContent (arrpetitfils);
								else if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "ppcp") == 0)
									pSharedMemory->wdata.days[i].part[j].cPrecipitationProba = xmlNodeGetContent (arrpetitfils);
							}
						}
					}
				}  // fin du jour n.
			}
		}
	}
	_cd_weather_close_xml_file (doc);
}


static void cd_weather_get_distant_data (CDSharedMemory *pSharedMemory)
{
	//\____________________ On recupere les conditions courantes sur le serveur.
	pSharedMemory->bErrorInThread = FALSE;
	GError *erreur = NULL;
	gchar *cCommand;
	gchar *cCCData = NULL;
	if (pSharedMemory->bCurrentConditions)
	{
		gchar *cURL = g_strdup_printf (CD_WEATHER_BASE_URL"/weather/local/%s?cc=*%s", pSharedMemory->cLocationCode, (pSharedMemory->bISUnits ? "&unit=m" : ""));
		cCCData = cairo_dock_get_url_data (cURL, &erreur);
		g_free (cURL);
		if (erreur != NULL)
		{
			cd_warning ("while downloading current conditions data:\n%s -> %s", cURL, erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		if (cCCData == NULL)
		{
			pSharedMemory->bErrorInThread = TRUE;
			return;  // a la 1ere erreur on quitte.
		}
	}
	
	//\____________________ On recupere les previsions a N jours sur le serveur.
	gchar *cForecastData = NULL;
	if (pSharedMemory->iNbDays > 0)
	{
		gchar *cURL = g_strdup_printf (CD_WEATHER_BASE_URL"/weather/local/%s?dayf=%d%s", pSharedMemory->cLocationCode, pSharedMemory->iNbDays, (pSharedMemory->bISUnits ? "&unit=m" : ""));
		cForecastData = cairo_dock_get_url_data (cURL, &erreur);
		g_free (cURL);
		if (erreur != NULL)
		{
			cd_warning ("while downloading forecast data:\n%s ->  %s", cURL, erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			pSharedMemory->bErrorInThread = TRUE;
		}
		if (cForecastData == NULL)
		{
			pSharedMemory->bErrorInThread = TRUE;
		}
	}
	
	//\____________________ On extrait les donnees des conditions courantes.
	if (cCCData != NULL)
	{
		_cd_weather_parse_data (pSharedMemory, cCCData, TRUE, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("weather : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			pSharedMemory->bErrorInThread = TRUE;
		}
		g_free (cCCData);
	}
	
	//\____________________ On extrait les donnees des previsions a N jours.
	if (cForecastData != NULL)
	{
		_cd_weather_parse_data (pSharedMemory, cForecastData, FALSE, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("weather : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			pSharedMemory->bErrorInThread = TRUE;
		}
		g_free (cForecastData);
	}
}



static void _reset_units (Unit *pUnits)
{
	xmlFree (pUnits->cTemp);
	xmlFree (pUnits->cDistance);
	xmlFree (pUnits->cSpeed);
	xmlFree (pUnits->cPressure);
}

static void _reset_current_conditions (CurrentContitions *pCurrentContitions)
{
	xmlFree (pCurrentContitions->cSunRise);
	xmlFree (pCurrentContitions->cSunSet);
	xmlFree (pCurrentContitions->cDataAcquisitionDate);
	xmlFree (pCurrentContitions->cObservatory);
	xmlFree (pCurrentContitions->cTemp);
	xmlFree (pCurrentContitions->cFeltTemp);
	xmlFree (pCurrentContitions->cWeatherDescription);
	xmlFree (pCurrentContitions->cIconNumber);
	xmlFree (pCurrentContitions->cWindSpeed);
	xmlFree (pCurrentContitions->cWindDirection);
	xmlFree (pCurrentContitions->cPressure);
	xmlFree (pCurrentContitions->cHumidity);
	xmlFree (pCurrentContitions->cMoonIconNumber);
}

static void _reset_current_one_day (Day *pDay)
{
	xmlFree (pDay->cName);
	xmlFree (pDay->cDate);
	xmlFree (pDay->cTempMax);
	xmlFree (pDay->cTempMin);
	xmlFree (pDay->cSunRise);
	xmlFree (pDay->cSunSet);
	int j;
	for (j = 0; j < 2; j ++)
	{
		xmlFree (pDay->part[j].cIconNumber);
		xmlFree (pDay->part[j].cWeatherDescription);
		xmlFree (pDay->part[j].cWindSpeed);
		xmlFree (pDay->part[j].cWindDirection);
		xmlFree (pDay->part[j].cHumidity);
		xmlFree (pDay->part[j].cPrecipitationProba);
	}
}

void cd_weather_reset_weather_data (CDWeatherData *pData)
{
	/**xmlFree (pData->cLon);
	xmlFree (pData->cLat);*/
	xmlFree (pData->cLocation);
	_reset_units (&pData->units);
	_reset_current_conditions (&pData->currentConditions);
	int i;
	for (i = 0; i < WEATHER_NB_DAYS_MAX; i ++)
	{
		_reset_current_one_day (&pData->days[i]);
	}
}

void cd_weather_reset_data (CairoDockModuleInstance *myApplet)
{
	cd_weather_reset_weather_data (&myData.wdata);
}

static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	cd_weather_reset_weather_data (&pSharedMemory->wdata);
	g_free (pSharedMemory);
}
void cd_weather_launch_periodic_task (CairoDockModuleInstance *myApplet)
{
	if (myData.pTask != NULL)
	{
		cairo_dock_discard_task (myData.pTask);
		myData.pTask = NULL;
	}
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->cLocationCode = g_strdup (myConfig.cLocationCode);
	pSharedMemory->bISUnits = myConfig.bISUnits;
	pSharedMemory->bCurrentConditions = myConfig.bCurrentConditions;
	pSharedMemory->iNbDays = myConfig.iNbDays;
	pSharedMemory->pApplet = myApplet;
	
	myData.pTask = cairo_dock_new_task_full (myConfig.iCheckInterval,
		(CairoDockGetDataAsyncFunc) cd_weather_get_distant_data,
		(CairoDockUpdateSyncFunc) cd_weather_update_from_data,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	cairo_dock_launch_task (myData.pTask);
}
