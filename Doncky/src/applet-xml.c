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
#include <math.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include <cairo-dock.h>
#include "applet-struct.h"
#include "applet-xml.h"
#include "applet-draw.h"


void cd_doncky_free_item (TextZone *pTextZone)
{
	if (pTextZone == NULL)
		return;
	
	g_free (pTextZone->cFont);
	g_free (pTextZone->cText);
	g_free (pTextZone->cCommand);
	g_free (pTextZone->cResult);
	g_free (pTextZone->cAlignWidth);
	g_free (pTextZone->cAlignHeight);
	g_free (pTextZone->cImgPath);
	g_free (pTextZone->cMountPoint);
	if (pTextZone->pImgSurface)
		cairo_surface_destroy (pTextZone->pImgSurface);
	g_free (pTextZone);
}


void cd_doncky_free_item_list (GldiModuleInstance *myApplet)
{
	if (myData.pTextZoneList == NULL)
		return;
	
	TextZone *pTextZone;	
	GList *t;
	for (t = myData.pTextZoneList; t != NULL; t = t->next)
	{
		pTextZone=  t->data;
		cd_doncky_free_item (pTextZone);
	}
	g_list_free (myData.pTextZoneList);
	myData.pTextZoneList = NULL;
}

gchar *_Get_FilePath (GldiModuleInstance *myApplet, const gchar *cXmlString)
{
	gchar *cString = (gchar *) cXmlString;
	gchar *cReturn;
	if (*cString == '~')
		cReturn = g_strdup_printf ("%s%s", getenv("HOME"), cString+1);
	else if (*cString == '/')
		cReturn = g_strdup (cString);
	else
	{
		if (g_str_has_suffix (cString, ".sh")
			|| g_str_has_suffix (cString, ".py")
			|| g_str_has_suffix (cString, ".png")
			|| g_str_has_suffix (cString, ".PNG")
			|| g_str_has_suffix (cString, ".jpg")
			|| g_str_has_suffix (cString, ".JPG")
			|| g_str_has_suffix (cString, ".jpeg")
			|| g_str_has_suffix (cString, ".JPEG")
			|| g_str_has_suffix (cString, ".svg")
			|| g_str_has_suffix (cString, ".SVG"))
		{
			cReturn = g_strdup_printf("%s%s", myData.cThemeFolder, cString);
		}
		else
			cReturn = g_strdup (cString);
	}
	return cReturn;
}


gboolean cd_doncky_readxml (GldiModuleInstance *myApplet)
{
	// On va lire le contenu de myConfig.cXmlFilePath	
	cd_debug ("Doncky-debug : ---------------------->  myConfig.cXmlFilePath = \"%s\"",myConfig.cXmlFilePath);
		
	g_return_val_if_fail (myConfig.cXmlFilePath != NULL, FALSE);
	xmlInitParser ();
	xmlDocPtr pXmlFile;
	xmlNodePtr pXmlMainNode;
	
	pXmlFile = cairo_dock_open_xml_file (myConfig.cXmlFilePath, "doncky", &pXmlMainNode, NULL);
	
	g_return_val_if_fail (pXmlFile != NULL && pXmlMainNode != NULL, FALSE);
	
	xmlChar *cNodeContent;
	TextZone *pTextZone = NULL;
		
	xmlNodePtr pXmlNode;
	
	myData.cPrevFont = g_strdup (myConfig.cDefaultFont);
	myData.cPrevAlignWidth = g_strdup_printf("left");
	myData.cPrevAlignHeight = g_strdup_printf("middle");
	myData.fPrevTextColor[0] = myConfig.fDefaultTextColor[0];
	myData.fPrevTextColor[1] = myConfig.fDefaultTextColor[1];
	myData.fPrevTextColor[2] = myConfig.fDefaultTextColor[2];
	myData.fPrevTextColor[3] = myConfig.fDefaultTextColor[3];
	
	// Récupération de myData.cThemeFolder et myData.cXmlFileName:
	myData.cThemeFolder = g_path_get_dirname (myConfig.cXmlFilePath);
	myData.cXmlFileName = g_path_get_basename (myConfig.cXmlFilePath);

	int i;
	for (pXmlNode = pXmlMainNode->children, i = 0; pXmlNode != NULL; pXmlNode = pXmlNode->next, i ++)
	{
		gchar *cContent = (gchar *)xmlNodeGetContent (pXmlNode);
		if (xmlStrcmp (pXmlNode->name, BAD_CAST "font") == 0)
		{
			g_free (myData.cPrevFont);
			myData.cPrevFont = (gchar *)xmlNodeGetContent (pXmlNode);
			if (strcmp (myData.cPrevFont, "default") == 0)
			{
				g_free (myData.cPrevFont);
				myData.cPrevFont = g_strdup (myConfig.cDefaultFont);
			}
		}
		
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "color") == 0)
		{
			if (strcmp (cContent, "default") == 0)
			{
				myData.fPrevTextColor[0] = myConfig.fDefaultTextColor[0];
				myData.fPrevTextColor[1] = myConfig.fDefaultTextColor[1];
				myData.fPrevTextColor[2] = myConfig.fDefaultTextColor[2];
				myData.fPrevTextColor[3] = myConfig.fDefaultTextColor[3];
			}
			else
			{
				cd_doncky_get_color_from_xml (cContent, myData.fPrevTextColor);
			}
		}
		
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "alignW") == 0)
		{
			g_free (myData.cPrevAlignWidth);
			myData.cPrevAlignWidth = (gchar *) xmlNodeGetContent (pXmlNode);
			if (strcmp (myData.cPrevAlignWidth, "left") == -1 && strcmp (myData.cPrevAlignWidth, "center") == -1 && strcmp (myData.cPrevAlignWidth, "right") == -1)
				myData.cPrevAlignWidth = g_strdup_printf("right");
		}
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "alignH") == 0)
		{
			g_free (myData.cPrevAlignHeight);
			myData.cPrevAlignHeight = (gchar *) xmlNodeGetContent (pXmlNode);
			if (strcmp (myData.cPrevAlignHeight, "top") == -1 && strcmp (myData.cPrevAlignHeight, "middle") == -1 && strcmp (myData.cPrevAlignHeight, "low") == -1)
				myData.cPrevAlignWidth = g_strdup_printf("middle");
		}
		
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "br") == 0 || xmlStrcmp (pXmlNode->name, BAD_CAST "nbr") == 0)
		{			
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			pTextZone->cFont = g_strdup (myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup (myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup (myData.cPrevAlignHeight);
			pTextZone->iRefresh = 0;
			pTextZone->cMountPoint = g_strdup_printf ("/");
			pTextZone->cCommand = NULL;
			pTextZone->bRefresh = FALSE;
			pTextZone->bBar = FALSE;
			
			pTextZone->iSpaceBetweenLines = g_strtod (cContent, NULL);
			
			if (xmlStrcmp (pXmlNode->name, BAD_CAST "br") == 0)
			{
				pTextZone->bEndOfLine = TRUE;
				pTextZone->bNextNewLine = TRUE;
			}
			else
			{
				pTextZone->bEndOfLine = TRUE;
				pTextZone->bNextNewLine = FALSE;
			}
		}
		
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "override") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			pTextZone->cFont = g_strdup (myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup (myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup (myData.cPrevAlignHeight);
			pTextZone->iRefresh = 0;
			pTextZone->cMountPoint = g_strdup_printf ("/");
			pTextZone->cCommand = NULL;
			pTextZone->cCommand = NULL;
			pTextZone->bRefresh = FALSE;
			pTextZone->bBar = FALSE;
			
			// On récupère le 1er champ -> = overrideH
			sscanf (cContent, "%d;%d", &(pTextZone->iOverrideH), &(pTextZone->iOverrideW));
		}
		
		
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "txt") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone); 	
			
			pTextZone->cText = (gchar *)xmlNodeGetContent (pXmlNode);
			pTextZone->cFont = g_strdup (myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup (myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup (myData.cPrevAlignHeight);
			pTextZone->iSpaceBetweenLines = 0;
			pTextZone->bEndOfLine = FALSE;
			pTextZone->bNextNewLine = FALSE;
			
			pTextZone->iRefresh = 0;
			pTextZone->cMountPoint = g_strdup_printf ("/");
			pTextZone->cCommand = NULL;
			pTextZone->bRefresh = FALSE;
			pTextZone->bBar = FALSE;
		}
		
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "stroke") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			pTextZone->cFont = g_strdup (myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			
			pTextZone->iHeight = atoi(cContent);
			
			myData.cPrevAlignWidth = g_strdup_printf("left");  // Sur toute la ligne -> On aligne forcément à gauche
			pTextZone->cAlignWidth = g_strdup (myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup (myData.cPrevAlignHeight);
			pTextZone->bBar = TRUE;			
			pTextZone->cText = g_strdup_printf("100"); // Une ligne est une barre avec une valeur toujours à 100 ;)
			pTextZone->bRefresh = FALSE;
				
		}
		
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "lstroke") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			pTextZone->cFont = g_strdup (myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];

			sscanf (cContent, "%d;%d", &(pTextZone->iWidth), &(pTextZone->iHeight));
			
			pTextZone->cAlignWidth = g_strdup (myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup (myData.cPrevAlignHeight);
			pTextZone->bLimitedBar = TRUE;			
			pTextZone->cText = g_strdup_printf("100"); // Une ligne est une barre avec une valeur toujours à 100 ;)
			pTextZone->bRefresh = FALSE;
		}
		
		
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "cmd") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			pTextZone->cFont = g_strdup (myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup (myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup (myData.cPrevAlignHeight);
			
			pTextZone->bBar = FALSE;
			
			pTextZone->cText = g_strdup (D_("Please wait...")); // On initialise le 1er texte à afficher à " "
			pTextZone->cMountPoint = g_strdup_printf ("/");
					
			xmlNodePtr pXmlSubNode;
			for (pXmlSubNode = pXmlNode->children; pXmlSubNode != NULL; pXmlSubNode = pXmlSubNode->next)
			{
				cNodeContent = xmlNodeGetContent (pXmlSubNode);
				
				if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "bash") == 0)
				{
					pTextZone->cCommand = _Get_FilePath (myApplet, (gchar *) cNodeContent);
					pTextZone->bIsBash = TRUE;
					pTextZone->bIsInternal = FALSE;
					pTextZone->bRefresh = TRUE;
					pTextZone->iRefresh = 0;
				}
				
				else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "echo") == 0)
				{
					// On insère sh -c 'echo " AVANT la commande et "' APRES
					if (*cNodeContent == '~')
						pTextZone->cCommand = g_strdup_printf ("sh -c 'echo \"%s%s\"'", getenv("HOME"), cNodeContent+1);
					else
						pTextZone->cCommand = g_strdup_printf ("sh -c 'echo \"%s\"'", cNodeContent);
					pTextZone->bIsBash = TRUE;
					pTextZone->bIsInternal = FALSE;
					pTextZone->bRefresh = TRUE;
					pTextZone->iRefresh = 0;
				}
				
				else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "internal") == 0)
				{
					pTextZone->cCommand = g_strdup ((gchar *)cNodeContent);
					pTextZone->bIsInternal = TRUE;
					pTextZone->bIsBash = FALSE;
					pTextZone->bRefresh = TRUE;
					pTextZone->iRefresh = 0;
				}
				
				
				else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "mount_point") == 0)
					pTextZone->cMountPoint = g_strdup ((gchar *)cNodeContent);
				
				
				else if ((xmlStrcmp (pXmlSubNode->name, BAD_CAST "lbar") == 0) || (xmlStrcmp (pXmlSubNode->name, BAD_CAST "bar") == 0))
				{
					// memperc;10
					// Récupération du 1er champ (commun aux Bars et aux LimitedBars:
					pTextZone->cCommand = g_strdup ((gchar *)cNodeContent);
					gchar *str = strchr (pTextZone->cCommand, ';');
					*str = '\0';
					str ++;
					pTextZone->bIsInternal = TRUE;
					pTextZone->bIsBash = FALSE;
					cd_debug ("##### %s => %s", pTextZone->cCommand, str);
					if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "lbar") == 0)
					{
						pTextZone->bLimitedBar = TRUE;
						sscanf (str, "%d;%d", &(pTextZone->iWidth), &(pTextZone->iHeight));
					}
					else
					{
						pTextZone->bBar = TRUE;
						// Récupération du 2eme champ pour les barres -> = Hauteur:
						pTextZone->iHeight = atoi(str);
						// Les Bars sont sur toute la ligne -> On aligne forcément à gauche
						myData.cPrevAlignWidth = g_strdup_printf("left");
						pTextZone->cAlignWidth = g_strdup (myData.cPrevAlignWidth);
						pTextZone->cAlignHeight = g_strdup (myData.cPrevAlignHeight);
					}
				}
				else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "refresh") == 0)
				{
					pTextZone->iRefresh = g_strtod ((gchar *) cNodeContent, NULL);
					pTextZone->bRefresh = TRUE;
				}
				xmlFree (cNodeContent);
			}
		}
		
		
		else if (xmlStrcmp (pXmlNode->name, BAD_CAST "img") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			
			pTextZone->cFont = g_strdup (myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup (myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup (myData.cPrevAlignHeight);
			
			pTextZone->bBar = FALSE;
			
			xmlNodePtr pXmlSubNode;			
			for (pXmlSubNode = pXmlNode->children; pXmlSubNode != NULL; pXmlSubNode = pXmlSubNode->next)
			{				
				cNodeContent = xmlNodeGetContent (pXmlSubNode);
				
				if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "file") == 0)
				{
					pTextZone->cImgPath = _Get_FilePath (myApplet, (gchar *)cNodeContent);
					pTextZone->bImgDraw=FALSE;
				}
				else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "size") == 0)
				{
					pTextZone->iImgSize = g_strtod ((gchar *)cNodeContent, NULL);
				}
				else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "sizeW") == 0)
				{
					pTextZone->iWidth= g_strtod ((gchar *)cNodeContent, NULL);
				}				
				else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "sizeH") == 0)
				{
					pTextZone->iHeight = g_strtod ((gchar *)cNodeContent, NULL);
				}
				xmlFree (cNodeContent);
			}
		}
		g_free (cContent);
	}
	
	cairo_dock_close_xml_file (pXmlFile);
			
	return TRUE;
}
