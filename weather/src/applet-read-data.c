/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "applet-struct.h"
#include "applet-read-data.h"

#define WEATHER_CURRENT_CONDITIONS_FILE "/cd-current_conditions"
#define WEATHER_FORECAST_FILE "/cd-forecast"

extern AppletConfig myConfig;
extern AppletData myData;


void cd_weather_get_data (gchar **cCurrentConditionsFilePath, gchar **cForecastFilePath)
{
	gboolean bTest = FALSE;
	if (bTest)
	{
		*cCurrentConditionsFilePath = g_strdup ("/opt/cairo-dock/trunk/plug-ins/weather/data/frxx0076.xml");
		*cForecastFilePath = g_strdup ("/opt/cairo-dock/trunk/plug-ins/weather/data/FRXX0076-meteo.xml");
		return ;
	}
	gchar *cCommand;
	if (myConfig.bCurrentConditions)
	{
		*cCurrentConditionsFilePath = g_strconcat (g_get_tmp_dir (), WEATHER_CURRENT_CONDITIONS_FILE, NULL);
		cCommand = g_strdup_printf ("wget \"http://xoap.weather.com/weather/local/%s?cc=*&prod=xoap&par=1048871467&key=12daac2f3a67cb39%s\" -O %s", myConfig.cLocationCode, (myConfig.bISUnits ? "&unit=m" : ""), *cCurrentConditionsFilePath);
		system (cCommand);
		g_free (cCommand);
	}
	
	if (myConfig.iNbDays > 0)
	{
		*cForecastFilePath = g_strconcat (g_get_tmp_dir (), WEATHER_FORECAST_FILE, NULL);
		cCommand = g_strdup_printf ("wget \"http://xoap.weather.com/weather/local/%s?dayf=%d&prod=xoap&par=1048871467&key=12daac2f3a67cb39%s\" -O %s", myConfig.cLocationCode, myConfig.iNbDays, (myConfig.bISUnits ? "&unit=m" : ""), *cForecastFilePath);
		system (cCommand);
		g_free (cCommand);
	}
}


void cd_weather_parse_data (gchar *cDataFilePath, gboolean bParseHeader, GError **erreur)
{
	cd_message ("");
	g_return_if_fail (g_file_test (cDataFilePath, G_FILE_TEST_EXISTS));
	
	GError *tmp_erreur = NULL;
	xmlDocPtr doc;
	xmlNodePtr noeud;
	xmlNodePtr param, fils, petitfils, arrpetitfils, arrarrpetitfils;
	gchar *nom, *visible, *name, *defaultsource = NULL, *source, *where;
	xmlChar *contenu;
	int i, j;
	gchar *index_str;
	
	xmlInitParser ();
	
	doc = xmlParseFile (cDataFilePath);
	if (doc == NULL)
	{
		cd_warning ("");
		return ;
	}
	
	noeud = xmlDocGetRootElement (doc);
	if ((noeud == NULL) || (xmlStrcmp (noeud->name, (const xmlChar *) "weather") != 0))
	{
		cd_warning ("");
		return ;
	}
	
	for (param = noeud->xmlChildrenNode; param != NULL; param = param->next)
	{
		if (bParseHeader && xmlStrcmp (param->name, (const xmlChar *) "head") == 0)
		{
			for (fils = param->children; fils != NULL; fils = fils->next)
			{
				if (xmlStrcmp (fils->name, (const xmlChar *) "ut") == 0)
					myData.units.cTemp = xmlNodeGetContent (fils);
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
					myData.days[i].cName = (gchar *) xmlGetProp (fils, (xmlChar *) "t");
					myData.days[i].cDate = (gchar *) xmlGetProp (fils, (xmlChar *) "dt");
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
										if (xmlStrcmp (petitfils->name, (const xmlChar *) "s") == 0)
											myData.days[i].part[j].cWindSpeed = xmlNodeGetContent (arrarrpetitfils);
										else if (xmlStrcmp (petitfils->name, (const xmlChar *) "t") == 0)
											myData.days[i].part[j].cWindDirection = xmlNodeGetContent (arrarrpetitfils);
									}
								}
								else if (xmlStrcmp (arrpetitfils->name, (const xmlChar *) "hmid") == 0)
									myData.days[i].part[j].cHumidity = xmlNodeGetContent (arrpetitfils);
							}
						}
					}
				}  // fin du jour n.
			}
		}
	}
	xmlCleanupParser ();
	xmlFreeDoc (doc);
}
