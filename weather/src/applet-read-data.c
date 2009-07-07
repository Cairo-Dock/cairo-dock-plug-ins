/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#define _BSD_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-read-data.h"

#define CD_WEATHER_BASE_URL "http://xml.weather.com"

gchar *cd_weather_get_location_data (gchar *cLocation)
{
	gchar *cLocationFilePath = g_strdup ("/tmp/weather-location.XXXXXX");
	int fds = mkstemp (cLocationFilePath);
	if (fds == -1)
	{
		g_free (cLocationFilePath);
		return NULL;
	}
	gchar *cCommand = g_strdup_printf ("wget \""CD_WEATHER_BASE_URL"/search/search?where=%s\" -O %s -o /dev/null -t 3 -T 10", cLocation, cLocationFilePath);
	int r = system (cCommand);
	g_free (cCommand);
	close(fds);
	return cLocationFilePath;
}


static xmlDocPtr _cd_weather_open_xml_file (gchar *cDataFilePath, xmlNodePtr *root_node, gchar *cRootNodeName, GError **erreur)
{
	if (cairo_dock_get_file_size (cDataFilePath) == 0)
	{
		g_set_error (erreur, 1, 1, "file '%s' doesn't exist (no connection ?)", cDataFilePath);
		return NULL;
	}
	xmlInitParser ();
	
	xmlDocPtr doc = xmlParseFile (cDataFilePath);
	if (doc == NULL)
	{
		g_set_error (erreur, 1, 1, "file '%s' is uncorrect (no connection ?)", cDataFilePath);
		return NULL;
	}
	
	xmlNodePtr noeud = xmlDocGetRootElement (doc);
	if (noeud == NULL || xmlStrcmp (noeud->name, (const xmlChar *) cRootNodeName) != 0)
	{
		g_set_error (erreur, 1, 2, "xml file '%s' is not well formed (weather.com may have changed its data format)", cDataFilePath);
		return doc;
	}
	*root_node = noeud;
	return doc;
}
static void _cd_weather_close_xml_file (xmlDocPtr doc)
{
	xmlCleanupParser ();
	xmlFreeDoc (doc);
}


GList *cd_weather_parse_location_data (gchar *cDataFilePath, GError **erreur)
{
	cd_message ("%s (%s)", __func__, cDataFilePath);
	
	GError *tmp_erreur = NULL;
	xmlNodePtr noeud;
	xmlDocPtr doc = _cd_weather_open_xml_file (cDataFilePath, &noeud, "search", &tmp_erreur);
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


static void _cd_weather_parse_data (CairoDockModuleInstance *myApplet, gchar *cDataFilePath, gboolean bParseHeader, GError **erreur)
{
	cd_message ("%s (%s)", __func__, cDataFilePath);
	
	GError *tmp_erreur = NULL;
	xmlNodePtr noeud;
	xmlDocPtr doc = _cd_weather_open_xml_file (cDataFilePath, &noeud, "weather", &tmp_erreur);
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
					if (degree == NULL || strncmp (degree, "°", strlen ("°")) != 0)
					{
						myData.units.cTemp = g_strconcat ("°", degree, NULL);
						g_free (degree);
					}
					else
						myData.units.cTemp = degree;
				}
				else if (xmlStrcmp (fils->name, (const xmlChar *) "ud") == 0)
					myData.units.cDistance = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "us") == 0)
					myData.units.cSpeed = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "up") == 0)
					myData.units.cPressure = xmlNodeGetContent (fils);
				//else if (xmlStrcmp (fils->name, (const xmlChar *) "ur") == 0)  // ?
				//	myData.units.cR = xmlNodeGetContent (fils);
			}
		}
		else if (bParseHeader && xmlStrcmp (param->name, (const xmlChar *) "loc") == 0)
		{
			for (fils = param->children; fils != NULL; fils = fils->next)
			{
				if (xmlStrcmp (fils->name, (const xmlChar *) "dnam") == 0)
					myData.cLocation = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "lat") == 0)
					myData.cLat = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "lon") == 0)
					myData.cLon = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "sunr") == 0)
					myData.currentConditions.cSunRise = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "suns") == 0)
					myData.currentConditions.cSunSet = xmlNodeGetContent (fils);
			}
		}
		else if (xmlStrcmp (param->name, (const xmlChar *) "cc") == 0)
		{
			for (fils = param->children; fils != NULL; fils = fils->next)
			{
				if (xmlStrcmp (fils->name, (const xmlChar *) "lsup") == 0)
					myData.currentConditions.cDataAcquisitionDate = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "obst") == 0)
					myData.currentConditions.cObservatory = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "tmp") == 0)
					myData.currentConditions.cTemp = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "flik") == 0)
					myData.currentConditions.cFeeledTemp = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "t") == 0)
					myData.currentConditions.cWeatherDescription = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "icon") == 0)
					myData.currentConditions.cIconNumber = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "wind") == 0)
				{
					for (petitfils = fils->children; petitfils != NULL; petitfils = petitfils->next)
					{
						if (xmlStrcmp (petitfils->name, (const xmlChar *) "s") == 0)
							myData.currentConditions.cWindSpeed = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "t") == 0)
							myData.currentConditions.cWindDirection = xmlNodeGetContent (petitfils);
					}
				}
				else if (xmlStrcmp (fils->name, (const xmlChar *) "bar") == 0)
				{
					for (petitfils = fils->children; petitfils != NULL; petitfils = petitfils->next)
					{
						if (xmlStrcmp (petitfils->name, (const xmlChar *) "r") == 0)
							myData.currentConditions.cPressure = xmlNodeGetContent (petitfils);
					}
				}
				else if (xmlStrcmp (fils->name, (const xmlChar *) "hmid") == 0)
					myData.currentConditions.cHumidity = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "moon") == 0)
				{
					for (petitfils = fils->children; petitfils != NULL; petitfils = petitfils->next)
					{
						if (xmlStrcmp (petitfils->name, (const xmlChar *) "icon") == 0)
							myData.currentConditions.cMoonIconNumber = xmlNodeGetContent (petitfils);
					}
				}
			}
		}
		else if (xmlStrcmp (param->name, (const xmlChar *) "dayf") == 0)
		{
			for (fils = param->children; fils != NULL; fils = fils->next)
			{
				if (xmlStrcmp (fils->name, (const xmlChar *) "lsup") == 0)
					myData.currentConditions.cDataAcquisitionDate = xmlNodeGetContent (fils);
				else if (xmlStrcmp (fils->name, (const xmlChar *) "day") == 0)
				{
					index_str = (gchar *) xmlGetProp (fils, (xmlChar *) "d");
					if (index_str == NULL)
						continue;
					i = atoi (index_str);
					g_free (index_str);
					cDayName = (gchar *) xmlGetProp (fils, (xmlChar *) "t");
					myData.days[i].cName = g_strdup (D_(cDayName));
					g_free (cDayName);
					cDate = (gchar *) xmlGetProp (fils, (xmlChar *) "dt");
					str = strchr (cDate, ' ');
					if (str != NULL)
					{
						*str = '\0';
						myData.days[i].cDate = g_strconcat (D_(cDate), " ", str+1, NULL);
						g_free (cDate);
					}
					else
						myData.days[i].cDate = cDate;
					for (petitfils = fils->children; petitfils != NULL; petitfils = petitfils->next)
					{
						if (xmlStrcmp (petitfils->name, (const xmlChar *) "hi") == 0)
							myData.days[i].cTempMax = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "low") == 0)
							myData.days[i].cTempMin = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "sunr") == 0)
							myData.days[i].cSunRise = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "suns") == 0)
							myData.days[i].cSunSet = xmlNodeGetContent (petitfils);
						else if (xmlStrcmp (petitfils->name, (const xmlChar *) "part") == 0)
						{
							index_str = (gchar *) xmlGetProp (petitfils, (xmlChar *) "p");
							if (index_str == NULL)
								continue;
							j = (*index_str == 'd' ? 0 : 1);  // jour : 0 / nuit : 1.
							for (arrpetitfils = petitfils->children; arrpetitfils != NULL; arrpetitfils = arrpetitfils->next)
							{
								if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "icon") == 0)
									myData.days[i].part[j].cIconNumber = xmlNodeGetContent (arrpetitfils);
								else if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "t") == 0)
									myData.days[i].part[j].cWeatherDescription = xmlNodeGetContent (arrpetitfils);
								else if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "wind") == 0)
								{
									for (arrarrpetitfils = arrpetitfils->children; arrarrpetitfils != NULL; arrarrpetitfils = arrarrpetitfils->next)
									{
										if (xmlStrcmp (arrarrpetitfils->name, (const xmlChar *) "s") == 0)
											myData.days[i].part[j].cWindSpeed = xmlNodeGetContent (arrarrpetitfils);
										else if (xmlStrcmp (arrarrpetitfils->name, (const xmlChar *) "t") == 0)
											myData.days[i].part[j].cWindDirection = xmlNodeGetContent (arrarrpetitfils);
									}
								}
								else if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "hmid") == 0)
									myData.days[i].part[j].cHumidity = xmlNodeGetContent (arrpetitfils);
								else if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "ppcp") == 0)
									myData.days[i].part[j].cPrecipitationProba = xmlNodeGetContent (arrpetitfils);
							}
						}
					}
				}  // fin du jour n.
			}
		}
	}
	_cd_weather_close_xml_file (doc);
}


void cd_weather_get_distant_data (CairoDockModuleInstance *myApplet)
{
	//\____________________ On recupere les conditions courantes sur le serveur.
	int r;
	gchar *cCommand;
	gchar *cCCDataFilePath = NULL;
	if (myConfig.bCurrentConditions)
	{
		cCCDataFilePath = g_strdup ("/tmp/weather-cc.XXXXXX");
		int fds = mkstemp (cCCDataFilePath);
		if (fds == -1)
		{
			g_free (cCCDataFilePath);
			return;
		}
		cCommand = g_strdup_printf ("wget \""CD_WEATHER_BASE_URL"/weather/local/%s?cc=*%s\" -O %s -o /dev/null -t 3 -T 10", myConfig.cLocationCode, (myConfig.bISUnits ? "&unit=m" : ""), cCCDataFilePath);  // &prod=xoap&par=1048871467&key=12daac2f3a67cb39
		cd_debug ("weather : %s", cCommand);
		r = system (cCommand);
		g_free (cCommand);
		close(fds);
	}
	
	//\____________________ On recupere les previsions a N jours sur le serveur.
	gchar *cForecastDataFilePath = NULL;
	if (myConfig.iNbDays > 0)
	{
		cForecastDataFilePath = g_strdup ("/tmp/weather-forecast.XXXXXX");
		int fds = mkstemp (cForecastDataFilePath);
		if (fds == -1)
		{
			g_free (cForecastDataFilePath);
			return;
		}
		cCommand = g_strdup_printf ("wget \""CD_WEATHER_BASE_URL"/weather/local/%s?dayf=%d%s\" -O %s -o /dev/null -t 3 -T 10", myConfig.cLocationCode, myConfig.iNbDays, (myConfig.bISUnits ? "&unit=m" : ""), cForecastDataFilePath);  // &prod=xoap&par=1048871467&key=12daac2f3a67cb39
		cd_debug ("weather : %s", cCommand);
		r = system (cCommand);
		g_free (cCommand);
		close(fds);
	}
	
	//\____________________ On extrait les donnees des conditions courantes.
	GError *erreur = NULL;
	if (cCCDataFilePath != NULL)
	{
		_cd_weather_parse_data (myApplet, cCCDataFilePath, TRUE, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("weather : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			myData.bErrorInThread = TRUE;
		}
		else
			myData.bErrorInThread = FALSE;
		g_remove (cCCDataFilePath);
		g_free (cCCDataFilePath);
	}
	
	//\____________________ On extrait les donnees des previsions a N jours.
	if (cForecastDataFilePath != NULL)
	{
		_cd_weather_parse_data (myApplet, cForecastDataFilePath, FALSE, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("weather : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
			myData.bErrorInThread = TRUE;
		}
		else
			myData.bErrorInThread = FALSE;
		g_remove (cForecastDataFilePath);
		g_free (cForecastDataFilePath);
	}
}
