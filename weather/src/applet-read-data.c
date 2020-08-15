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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>  // rand

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-read-data.h"

// Credentials used by Cairo-Dock for Yahoo weather
// please do not use them for any other purpose than this applet
#define APP_ID "kv8SljKW"
#define CLIENT_ID "dj0yJmk9N3d2dVZVUHIxWVVpJnM9Y29uc3VtZXJzZWNyZXQmc3Y9MCZ4PWM0"
#define CLIENT_SECRET "'2c776c9ee22643fec:59g53568d92142f97ec12e'"

static gchar *_get_oauth_header (const gchar *cLocationName, gboolean bISUnits);

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
	if (noeud == NULL || xmlStrcmp (noeud->name, BAD_CAST cRootNodeName) != 0)
	{
		g_set_error (erreur, 1, 2, "xml data is not well formed (yahoo.com may have changed its data format)");
		return doc;
	}
	*root_node = noeud;
	return doc;
}
/* Not used
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
*/
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
		if (xmlStrcmp (param->name, BAD_CAST "loc") == 0)
		{
			cLocationsList = g_list_prepend (cLocationsList, xmlNodeGetContent (param));
			cLocationsList = g_list_prepend (cLocationsList,  xmlGetProp (param, BAD_CAST "id"));
		}
	}
	_cd_weather_close_xml_file (doc);
	return cLocationsList;
}


static void _cd_weather_parse_data (CDSharedMemory *pSharedMemory, const gchar *cData, gboolean bParseHeader, GError **erreur)
{
	GError *tmp_erreur = NULL;
	xmlNodePtr noeud = NULL;
	xmlDocPtr doc = _cd_weather_open_xml_buffer (cData, &noeud, "rss", &tmp_erreur);
	if (tmp_erreur != NULL)
	{
		g_propagate_error (erreur, tmp_erreur);
		_cd_weather_close_xml_file (doc);
		return ;
	}
	
	xmlNodePtr rss_child, channel_child, item_child;
	int i=0;  // day counter
	for (rss_child = noeud->xmlChildrenNode; rss_child != NULL; rss_child = rss_child->next)
	{
		if (xmlStrcmp (rss_child->name, BAD_CAST "channel") == 0)
		{
			for (channel_child = rss_child->children; channel_child != NULL; channel_child = channel_child->next)
			{
				if (xmlStrcmp (channel_child->name, BAD_CAST "units") == 0)
				{
					pSharedMemory->wdata.units.cTemp = xmlGetProp (channel_child, BAD_CAST "temperature");
					pSharedMemory->wdata.units.cSpeed = xmlGetProp (channel_child, BAD_CAST "speed");
					pSharedMemory->wdata.units.cPressure = xmlGetProp (channel_child, BAD_CAST "pressure");
				}
				else if (xmlStrcmp (channel_child->name, BAD_CAST "link") == 0)
				{
					pSharedMemory->wdata.cLink = xmlNodeGetContent (channel_child);
				}
				else if (xmlStrcmp (channel_child->name, BAD_CAST "location") == 0)
				{
					pSharedMemory->wdata.cCity = xmlGetProp (channel_child, BAD_CAST "city");
					pSharedMemory->wdata.cCountry = xmlGetProp (channel_child, BAD_CAST "country");
				}
				else if (xmlStrcmp (channel_child->name, BAD_CAST "wind") == 0)
				{
					pSharedMemory->wdata.currentConditions.cWindDirection = xmlGetProp (channel_child, BAD_CAST "direction");
					pSharedMemory->wdata.currentConditions.cWindSpeed = xmlGetProp (channel_child, BAD_CAST "speed");
				}
				else if (xmlStrcmp (channel_child->name, BAD_CAST "atmosphere") == 0)
				{
					pSharedMemory->wdata.currentConditions.cHumidity = xmlGetProp (channel_child, BAD_CAST "humidity");
					pSharedMemory->wdata.currentConditions.cPressure = xmlGetProp (channel_child, BAD_CAST "pressure");
				}
				else if (xmlStrcmp (channel_child->name, BAD_CAST "astronomy") == 0)
				{
					pSharedMemory->wdata.currentConditions.cSunRise = xmlGetProp (channel_child, BAD_CAST "sunrise");
					pSharedMemory->wdata.currentConditions.cSunSet = xmlGetProp (channel_child, BAD_CAST "sunset");
				}
				else if (xmlStrcmp (channel_child->name, BAD_CAST "ttl") == 0)
				{
					xmlChar *ttl = xmlNodeGetContent (channel_child);
					pSharedMemory->wdata.currentConditions.ttl = ttl ? atoi((gchar*)ttl) : 0;
					xmlFree (ttl);
				}
				else if (xmlStrcmp (channel_child->name, BAD_CAST "lastBuildDate") == 0)
				{
					pSharedMemory->wdata.currentConditions.cDataAcquisitionDate = xmlNodeGetContent (channel_child);
				}
				else if (xmlStrcmp (channel_child->name, BAD_CAST "item") == 0)
				{
					for (item_child = channel_child->children; item_child != NULL; item_child = item_child->next)
					{
						if (xmlStrcmp (item_child->name, BAD_CAST "condition") == 0)
						{
							pSharedMemory->wdata.currentConditions.now.cIconNumber = xmlGetProp (item_child, BAD_CAST "code");
							pSharedMemory->wdata.currentConditions.now.cDate = xmlGetProp (item_child, BAD_CAST "date");
							pSharedMemory->wdata.currentConditions.now.cTempMax = xmlGetProp (item_child, BAD_CAST "temp");
							pSharedMemory->wdata.currentConditions.now.cWeatherDescription = xmlGetProp (item_child, BAD_CAST "text");
						}
						else if (xmlStrcmp (item_child->name, BAD_CAST "forecast") == 0 && i<WEATHER_NB_DAYS_MAX)
						{
							pSharedMemory->wdata.days[i].cIconNumber = xmlGetProp (item_child, BAD_CAST "code");
							pSharedMemory->wdata.days[i].cDate = xmlGetProp (item_child, BAD_CAST "date");
							pSharedMemory->wdata.days[i].cName = xmlGetProp (item_child, BAD_CAST "day");
							pSharedMemory->wdata.days[i].cTempMax = xmlGetProp (item_child, BAD_CAST "high");
							pSharedMemory->wdata.days[i].cTempMin = xmlGetProp (item_child, BAD_CAST "low");
							pSharedMemory->wdata.days[i].cWeatherDescription = xmlGetProp (item_child, BAD_CAST "text");
							i++;
						}
					}
				}
			}
		}
	}
	_cd_weather_close_xml_file (doc);
}

const gchar *cBaseUrl = "https://weather-ydn-yql.media.yahoo.com/forecastrss";
const gchar *cBaseUrlEsc = "https%3A%2F%2Fweather-ydn-yql.media.yahoo.com%2Fforecastrss";
	
static void cd_weather_get_distant_data (CDSharedMemory *pSharedMemory)
{
	pSharedMemory->bErrorInThread = FALSE;
	
	//\____________________ remove spaces, and add a comma if necessary
	gchar *r = strchr(pSharedMemory->cLocationCode, ' '), *w = r;
	if (r)
	{
		const gchar *r0 = pSharedMemory->cLocationCode;  // just for the sake of readability
		while (*r)
		{
			if (*r == ' ')
			{
				if (w != r0 && *(w-1) != ',')
					*w++ = ',';
			}
			else
			{
				if (*r != ',' || (w != r0 && *(w-1) != ','))
					*w++ = *r;
			}
			r++;
		}
		*w = '\0';
	}
	cd_debug("location code: %s", pSharedMemory->cLocationCode);
	
	//\____________________ get the rss feed
	GError *erreur = NULL;
	gchar *cURL = g_strdup_printf("%s?location=%s&format=xml&u=%c", cBaseUrl, pSharedMemory->cLocationCode, pSharedMemory->bISUnits ? 'c' : 'f');
	gchar *oauth_header = _get_oauth_header (pSharedMemory->cLocationCode, pSharedMemory->bISUnits);
	gchar *cCCData = cairo_dock_get_url_data_with_headers (cURL, FALSE, &erreur,
		"Authorization", oauth_header,
		"X-Yahoo-App-Id", APP_ID,
		NULL);
	if (erreur != NULL)
	{
		cd_warning ("while downloading current conditions data:\n%s -> %s", cURL, erreur->message);
		g_error_free (erreur);
		erreur = NULL;
	}
	g_free (oauth_header);
	g_free (cURL);
	
	if (cCCData == NULL)
	{
		pSharedMemory->bErrorInThread = TRUE;
		return;
	}
	
	//\____________________ On extrait les donnees des conditions courantes.
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



static void _reset_units (Unit *pUnits)
{
	xmlFree (pUnits->cTemp);
	xmlFree (pUnits->cSpeed);
	xmlFree (pUnits->cPressure);
}

static void _reset_current_conditions (CurrentContitions *pCurrentContitions)
{
	xmlFree (pCurrentContitions->cSunRise);
	xmlFree (pCurrentContitions->cSunSet);
	xmlFree (pCurrentContitions->cDataAcquisitionDate);
	xmlFree (pCurrentContitions->cWindSpeed);
	xmlFree (pCurrentContitions->cWindDirection);
	xmlFree (pCurrentContitions->cPressure);
	xmlFree (pCurrentContitions->cHumidity);
}

static void _reset_one_day (Day *pDay)
{
	xmlFree (pDay->cName);
	xmlFree (pDay->cDate);
	xmlFree (pDay->cTempMax);
	xmlFree (pDay->cTempMin);
	xmlFree (pDay->cIconNumber);
}

void cd_weather_reset_weather_data (CDWeatherData *pData)
{
	xmlFree (pData->cCity);
	xmlFree (pData->cCountry);
	_reset_units (&pData->units);
	_reset_current_conditions (&pData->currentConditions);
	int i;
	for (i = 0; i < WEATHER_NB_DAYS_MAX; i ++)
	{
		_reset_one_day (&pData->days[i]);
	}
}

void cd_weather_reset_data (GldiModuleInstance *myApplet)
{
	cd_weather_reset_weather_data (&myData.wdata);
}

static void _free_shared_memory (CDSharedMemory *pSharedMemory)
{
	cd_weather_reset_weather_data (&pSharedMemory->wdata);
	g_free (pSharedMemory);
}
void cd_weather_launch_periodic_task (GldiModuleInstance *myApplet)
{
	if (myData.pTask != NULL)
	{
		gldi_task_discard (myData.pTask);
		myData.pTask = NULL;
	}
	
	CDSharedMemory *pSharedMemory = g_new0 (CDSharedMemory, 1);
	pSharedMemory->cLocationCode = g_strdup (myConfig.cLocationCode);
	pSharedMemory->bISUnits = myConfig.bISUnits;
	pSharedMemory->bCurrentConditions = myConfig.bCurrentConditions;
	pSharedMemory->pApplet = myApplet;
	
	myData.pTask = gldi_task_new_full (3600,  // start with 1h period, it will be adjusted according to the TTL
		(GldiGetDataAsyncFunc) cd_weather_get_distant_data,
		(GldiUpdateSyncFunc) cd_weather_update_from_data,
		(GFreeFunc) _free_shared_memory,
		pSharedMemory);
	gldi_task_launch (myData.pTask);
}

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <sys/time.h>

static guchar *_hmac_sha1_base64(const gchar *key, const gchar *data)
{
	size_t key_len = strlen(++key);
	size_t data_len = strlen(data);
	uint8_t md[64];  // SHA1_Message_Block_Size = 64 bytes
	uint md_len=64;
	if (!HMAC(EVP_sha1(), key, key_len, (guchar*)data, data_len, md, &md_len))  // data is URL-encoded, so is actually a guchar*
	{
		//#include <openssl/err.h>
		//ERR_clear_error();
	}
	guchar *b64_output = malloc((md_len+1)*4/3+1);  // taking into account the padding, and the final '\0' inserted by the function
	EVP_EncodeBlock(b64_output, md, md_len);
	return b64_output;
}

static gchar *_get_oauth_header (const gchar *cLocationName, gboolean bISUnits)
{
	// Oauth authentication: the idea is to join to the headers a hash of the GET request's signature
	// the signature is made of: the method (GET), the url, some query parameters (like location, units and format), and the various oauth parameters (except of course the hash)
	// they must be ordered, so that both end agree on how to form the string and calculate the hash
	
	// first build the parameters string
	gchar *cLocationNameEsc = g_uri_escape_string(cLocationName, NULL, FALSE);
	struct timeval tv;
	gettimeofday(&tv, NULL);
	const gchar *nonce = g_strdup_printf("%032d", rand());  // 32 bytes; it's not like we really need to be secretive...
	gchar *params_sorted = g_strdup_printf ("format=xml&location=%s&oauth_consumer_key=%s&oauth_nonce=%s&oauth_signature_method=%s&oauth_timestamp=%ld&oauth_version=%s&u=%c",
		cLocationNameEsc,
		CLIENT_ID,
		nonce,
		"HMAC-SHA1",
		tv.tv_sec,
		"1.0",
		bISUnits ? 'c' : 'f');  // all parameters are sorted by their key, and their value is URL-encoded.
	gchar *params_sorted_esc = g_uri_escape_string(params_sorted, NULL, FALSE);
	// then build the request's signature string
	gchar *signature_base_str = g_strdup_printf ("GET&%s&%s",
		cBaseUrlEsc,
		params_sorted_esc);
	// now calculate the hash of the signature
	gchar *key = g_strdup (CLIENT_SECRET), *k=key;
	while (*k) (*k++)--;
	guchar *oauth_signature = _hmac_sha1_base64(key, signature_base_str);
	g_free (key);
	
	// finally build the oauth header, containing all the oauth parameters
	gchar *auth_header = g_strdup_printf("OAuth oauth_consumer_key=\"%s\", oauth_nonce=\"%s\", oauth_signature_method=\"%s\", oauth_timestamp=\"%ld\", oauth_version=\"%s\", oauth_signature=\"%s\"",
		CLIENT_ID,
		nonce,
		"HMAC-SHA1",
		tv.tv_sec,
		"1.0",
		oauth_signature);
	free(signature_base_str);
	free(cLocationNameEsc);
	free(oauth_signature);
	return auth_header;
}
