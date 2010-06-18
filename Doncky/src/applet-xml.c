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
	
	//g_free (pTextZone->cFont);  // Pourquoi çà plante ??
	g_free (pTextZone->cText);
	g_free (pTextZone->cCommand);
	g_free (pTextZone);	
}


void cd_doncky_free_item_list (CairoDockModuleInstance *myApplet)
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


gboolean cd_doncky_readxml (CairoDockModuleInstance *myApplet)
{
	// On va lire le contenu de myConfig.cXmlFilePath	
	cd_debug ("Doncky-debug : ---------------------->  myConfig.cXmlFilePath = \"%s\"",myConfig.cXmlFilePath);
		
	g_return_val_if_fail (myConfig.cXmlFilePath != NULL, FALSE);
	xmlInitParser ();
	xmlDocPtr pXmlFile;
	xmlNodePtr pXmlMainNode;
	
	pXmlFile = cairo_dock_open_xml_file (myConfig.cXmlFilePath, "doncky", &pXmlMainNode, NULL);
	
	g_return_val_if_fail (pXmlFile != NULL && pXmlMainNode != NULL, FALSE);
	
	
	xmlAttrPtr ap;
	xmlChar *cAttribute, *cNodeContent, *cTextNodeContent;
	TextZone *pTextZone = NULL;
		
	xmlNodePtr pXmlNode;
	
	myData.cPrevFont = g_strdup_printf("%s", myConfig.cDefaultFont);
	myData.cPrevAlignWidth = g_strdup_printf("left");
	myData.cPrevAlignHeight = g_strdup_printf("middle");
	myData.fPrevTextColor[0] = myConfig.fDefaultTextColor[0];
	myData.fPrevTextColor[1] = myConfig.fDefaultTextColor[1];
	myData.fPrevTextColor[2] = myConfig.fDefaultTextColor[2];
	myData.fPrevTextColor[3] = myConfig.fDefaultTextColor[3];
	
		
	int i;
	for (pXmlNode = pXmlMainNode->children, i = 0; pXmlNode != NULL; pXmlNode = pXmlNode->next, i ++)
	{
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "font") == 0)
		{
			myData.cPrevFont = xmlNodeGetContent (pXmlNode);			
			if (strcmp (myData.cPrevFont, "default") == 0)
				myData.cPrevFont = g_strdup_printf("%s", myConfig.cDefaultFont);
		}
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "color") == 0)
		{
			gchar *cTempo = xmlNodeGetContent (pXmlNode);
			
			if (strcmp (cTempo, "default") == 0)
			{
				myData.fPrevTextColor[0] = myConfig.fDefaultTextColor[0];
				myData.fPrevTextColor[1] = myConfig.fDefaultTextColor[1];
				myData.fPrevTextColor[2] = myConfig.fDefaultTextColor[2];
				myData.fPrevTextColor[3] = myConfig.fDefaultTextColor[3];
			}
			else
			{
				// On récupère le 1er champ -> red
				g_strreverse (cTempo);
				cTempo = strrchr(cTempo, ';');
				ltrim( cTempo, ";" );
				g_strreverse (cTempo);
				myData.fPrevTextColor[0] = atof(cTempo) / 255;
				
				// On récupère le 2ème champ -> = green
				cTempo = strchr(xmlNodeGetContent (pXmlNode), ';');
				ltrim( cTempo, ";" );
				g_strreverse (cTempo);
				cTempo = strchr(cTempo, ';');
				ltrim( cTempo, ";" );
				g_strreverse (cTempo);
				myData.fPrevTextColor[1] = atof(cTempo) / 255;
				
				// On récupère le 3ème champ -> = blue
				cTempo = strchr(xmlNodeGetContent (pXmlNode), ';');
				ltrim( cTempo, ";" );
				cTempo = strchr(cTempo, ';');
				ltrim( cTempo, ";" );
				g_strreverse (cTempo);
				cTempo = strchr(cTempo, ';');
				ltrim( cTempo, ";" );
				g_strreverse (cTempo);
				myData.fPrevTextColor[2] = atof(cTempo) / 255;
				
				// On récupère le dernier champ -> alpha
				cTempo = strrchr(xmlNodeGetContent (pXmlNode), ';');
				ltrim( cTempo, ";" );
				myData.fPrevTextColor[3] = atof(cTempo) / 255;
			}
		}
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "alignW") == 0)
		{
			myData.cPrevAlignWidth = xmlNodeGetContent (pXmlNode);
			if (strcmp (myData.cPrevAlignWidth, "left") == -1 && strcmp (myData.cPrevAlignWidth, "center") == -1 && strcmp (myData.cPrevAlignWidth, "right") == -1)
				myData.cPrevAlignWidth = g_strdup_printf("right");
		}
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "alignH") == 0)
		{
			myData.cPrevAlignHeight = xmlNodeGetContent (pXmlNode);
			if (strcmp (myData.cPrevAlignHeight, "top") == -1 && strcmp (myData.cPrevAlignHeight, "middle") == -1 && strcmp (myData.cPrevAlignHeight, "low") == -1)
				myData.cPrevAlignWidth = g_strdup_printf("middle");
		}
		
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "br") == 0 || xmlStrcmp (pXmlNode->name, (const xmlChar *) "nbr") == 0)
		{			
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			pTextZone->cText = g_strdup_printf("");
			pTextZone->cFont = g_strdup_printf("%s", myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup_printf("%s", myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup_printf("%s", myData.cPrevAlignHeight);
			pTextZone->iRefresh = 0;
			pTextZone->cMountPoint = g_strdup_printf ("/");
			pTextZone->cCommand = NULL;																
			pTextZone->cCommand = NULL;
			pTextZone->bRefresh = FALSE;
			pTextZone->bBar = FALSE;
			
		
			pTextZone->iSpaceBetweenLines = g_strtod (xmlNodeGetContent (pXmlNode), NULL);
			
			if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "br") == 0)
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
		
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "override") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			pTextZone->cText = g_strdup_printf("");
			pTextZone->cFont = g_strdup_printf("%s", myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup_printf("%s", myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup_printf("%s", myData.cPrevAlignHeight);
			pTextZone->iRefresh = 0;
			pTextZone->cMountPoint = g_strdup_printf ("/");
			pTextZone->cCommand = NULL;																
			pTextZone->cCommand = NULL;
			pTextZone->bRefresh = FALSE;
			pTextZone->bBar = FALSE;
			
			
			gchar *cTempo = xmlNodeGetContent (pXmlNode);			
			// On récupère le 1er champ -> = overrideH
			g_strreverse (cTempo);
			cTempo = strrchr(cTempo, ';');
			ltrim( cTempo, ";" );
			g_strreverse (cTempo);
			pTextZone->iOverrideH = atoi(cTempo);			
			// On récupère le dernier champ -> = overrideW
			cTempo = strrchr(xmlNodeGetContent (pXmlNode), ';');
			ltrim( cTempo, ";" );
			pTextZone->iOverrideW = atoi(cTempo);
		}
		
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "txt") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone); 	
			
			
			pTextZone->cText = xmlNodeGetContent (pXmlNode);
			pTextZone->cFont = g_strdup_printf("%s", myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup_printf("%s", myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup_printf("%s", myData.cPrevAlignHeight);
			pTextZone->iSpaceBetweenLines = 0;
			pTextZone->bEndOfLine = FALSE;
			pTextZone->bNextNewLine = FALSE;
			
			
			pTextZone->iRefresh = 0;
			pTextZone->cMountPoint = g_strdup_printf ("/");
			pTextZone->cCommand = NULL;
			pTextZone->bRefresh = FALSE;
			pTextZone->bBar = FALSE;
		}
		
		
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "stroke") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			pTextZone->cFont = g_strdup_printf("%s", myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
					
			
			gchar *cTempo = xmlNodeGetContent (pXmlNode);
			pTextZone->iHeight = atoi(cTempo);
						
			myData.cPrevAlignWidth = g_strdup_printf("left");  // Sur toute la ligne -> On aligne forcément à gauche
			pTextZone->cAlignWidth = g_strdup_printf("%s", myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup_printf("%s", myData.cPrevAlignHeight);
			pTextZone->bBar = TRUE;			
			pTextZone->cText = g_strdup_printf("100"); // Une ligne est une barre avec une valeur toujours à 100 ;)
			pTextZone->bRefresh = FALSE;
				
		}
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "lstroke") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			
			pTextZone->cFont = g_strdup_printf("%s", myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			
			gchar *cTempo = xmlNodeGetContent (pXmlNode);
			// On récupère le 1er champ -> = Largeur
			g_strreverse (cTempo);
			cTempo = strrchr(cTempo, ';') ;
			ltrim( cTempo, ";" );
			g_strreverse (cTempo);
			pTextZone->iWidth = atoi(cTempo);			
			// On récupère le 2ème champ -> = Hauteur
			cTempo = strrchr(xmlNodeGetContent (pXmlNode), ';') ;
			ltrim( cTempo, ";" );
			pTextZone->iHeight = atoi(cTempo);
			
			pTextZone->cAlignWidth = g_strdup_printf("%s", myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup_printf("%s", myData.cPrevAlignHeight);
			pTextZone->bLimitedBar = TRUE;			
			pTextZone->cText = g_strdup_printf("100"); // Une ligne est une barre avec une valeur toujours à 100 ;)
			pTextZone->bRefresh = FALSE;
		}
		
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "cmd") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			
			pTextZone->cFont = g_strdup_printf("%s", myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup_printf("%s", myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup_printf("%s", myData.cPrevAlignHeight);
			
			pTextZone->bBar = FALSE;
			
			
			pTextZone->cText = g_strdup_printf("Please wait... "); // On initialise le 1er texte à afficher à " "
			pTextZone->cMountPoint = g_strdup_printf ("/");
					
			xmlNodePtr pXmlSubNode;			
			for (pXmlSubNode = pXmlNode->children; pXmlSubNode != NULL; pXmlSubNode = pXmlSubNode->next)
			{				
				cNodeContent = xmlNodeGetContent (pXmlSubNode);
				
				
				
				
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "bash") == 0)
				{
					pTextZone->cCommand = g_strdup_printf("%s",g_str_replace (xmlNodeGetContent (pXmlSubNode), "~", g_strdup_printf("/home/%s", getenv("USER"))));
					pTextZone->bIsBash = TRUE;
					pTextZone->bIsInternal = FALSE;
				}
				
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "echo") == 0)
				{
					// On insère sh -c 'echo " AVANT la commande et "' APRES
					gchar *cXmlCommand;
					cXmlCommand = xmlNodeGetContent (pXmlSubNode);
					
					GString *sTemp =  g_string_new  ("");
					g_string_printf (sTemp, "sh -c 'echo \"%s\"'", cXmlCommand);
					pTextZone->cCommand = g_strdup_printf("%s",g_str_replace (sTemp->str, "~", g_strdup_printf("/home/%s", getenv("USER"))));
					
					g_string_free (sTemp, TRUE);
					g_free (cXmlCommand);
					pTextZone->bIsBash = TRUE;
					pTextZone->bIsInternal = FALSE;
				}
				
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "internal") == 0)
				{
					pTextZone->cCommand = g_strdup_printf("%s",g_str_replace (xmlNodeGetContent (pXmlSubNode), "~", g_strdup_printf("/home/%s", getenv("USER"))));
					pTextZone->bIsInternal = TRUE;
					pTextZone->bIsBash = FALSE;
				}
				
				
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "mount_point") == 0)
					pTextZone->cMountPoint = xmlNodeGetContent (pXmlSubNode);
				
				
				if ((xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "lbar") == 0) || (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "bar") == 0))
				{
					gchar *cTempo = xmlNodeGetContent (pXmlSubNode);

					// On récupère le 1er champ -> = commande interne
					g_strreverse (cTempo);
					cTempo = strrchr(cTempo, ';') ;
					ltrim( cTempo, ";" );
					g_strreverse (cTempo);
					pTextZone->cCommand = g_strdup_printf("%s",g_str_replace (cTempo, "~", g_strdup_printf("/home/%s", getenv("USER"))));
					pTextZone->bIsInternal = TRUE;
					pTextZone->bIsBash = FALSE;
					
					// On récupère le dernier champ -> = Hauteur
					cTempo = strrchr(xmlNodeGetContent (pXmlSubNode), ';') ;
					ltrim( cTempo, ";" );					
					pTextZone->iHeight = atoi(cTempo);
					
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "lbar") == 0)
					{					
						// On récupère le 2ème champ -> = Largeur
						cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';') ;
						ltrim( cTempo, ";" );
						g_strreverse (cTempo);					
						cTempo = strchr(cTempo, ';') ;			
						ltrim( cTempo, ";" );
						g_strreverse (cTempo);										
						pTextZone->iWidth = atoi(cTempo);
						
						pTextZone->bLimitedBar = TRUE;
					}
					else
					{
						myData.cPrevAlignWidth = g_strdup_printf("left");  // Sur toute la ligne -> On aligne forcément à gauche
						pTextZone->cAlignWidth = g_strdup_printf("%s", myData.cPrevAlignWidth);
						pTextZone->cAlignHeight = g_strdup_printf("%s", myData.cPrevAlignHeight);
						pTextZone->bBar = TRUE;
					}			
				}
				
				
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "refresh") == 0)
				{
					pTextZone->iRefresh = g_strtod (cNodeContent, NULL);
					pTextZone->bRefresh = TRUE;				
				}
				else
				{
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "bash") == 0 || xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "echo") == 0 || xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "internal") == 0)
					{
						pTextZone->bRefresh = TRUE;
						pTextZone->iRefresh = 0;
					}
				}
				xmlFree (cNodeContent);
			}
		}
		
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "img") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone);
			
			
			pTextZone->cFont = g_strdup_printf("%s", myData.cPrevFont);
			pTextZone->fTextColor[0] = myData.fPrevTextColor[0];
			pTextZone->fTextColor[1] = myData.fPrevTextColor[1];
			pTextZone->fTextColor[2] = myData.fPrevTextColor[2];
			pTextZone->fTextColor[3] = myData.fPrevTextColor[3];
			pTextZone->cAlignWidth = g_strdup_printf("%s", myData.cPrevAlignWidth);
			pTextZone->cAlignHeight = g_strdup_printf("%s", myData.cPrevAlignHeight);
			
			pTextZone->bBar = FALSE;
			
			xmlNodePtr pXmlSubNode;			
			for (pXmlSubNode = pXmlNode->children; pXmlSubNode != NULL; pXmlSubNode = pXmlSubNode->next)
			{				
				cNodeContent = xmlNodeGetContent (pXmlSubNode);
				
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "file") == 0)
				{
					pTextZone->cImgPath = g_strdup_printf("%s",g_str_replace (xmlNodeGetContent (pXmlSubNode), "~", g_strdup_printf("/home/%s", getenv("USER"))));
					pTextZone->bImgDraw=FALSE;
				}
				
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "size") == 0)
				{
					pTextZone->iImgSize = g_strtod (cNodeContent, NULL);				
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "sizeW") == 0)
				{
					pTextZone->iWidth= g_strtod (cNodeContent, NULL);				
				}				
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "sizeH") == 0)
				{
					pTextZone->iHeight = g_strtod (cNodeContent, NULL);				
				}
				xmlFree (cNodeContent);
			}
		}		
	}
	
	cairo_dock_close_xml_file (pXmlFile);
			
	return TRUE;
}
