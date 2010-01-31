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
	
	
	pXmlFile = cairo_dock_open_xml_file (myConfig.cXmlFilePath, "code", &pXmlMainNode, NULL);
	
	g_return_val_if_fail (pXmlFile != NULL && pXmlMainNode != NULL, FALSE);
		
	
	xmlAttrPtr ap;
	xmlChar *cAttribute, *cNodeContent, *cTextNodeContent;
	TextZone *pTextZone = NULL;
		
	xmlNodePtr pXmlNode;
	
	int i;
	for (pXmlNode = pXmlMainNode->children, i = 0; pXmlNode != NULL; pXmlNode = pXmlNode->next, i ++)
	{
		
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "zone") == 0)
		{
			pTextZone = g_new0 (TextZone, 1);
			myData.pTextZoneList = g_list_append (myData.pTextZoneList, pTextZone); 
			
			pTextZone->cCommand = NULL;
			pTextZone->bRefresh = FALSE;
			pTextZone->bEndOfLine = FALSE;	
			pTextZone->bBar = FALSE;
			
			
			xmlNodePtr pXmlSubNode;			
			for (pXmlSubNode = pXmlNode->children; pXmlSubNode != NULL; pXmlSubNode = pXmlSubNode->next)
			{				
				cNodeContent = xmlNodeGetContent (pXmlSubNode);
				
				// On gère le refresh à part pour lui imposer à 0 si rien n'est renseigné
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "refresh") == 0)
				{
					pTextZone->iRefresh = g_strtod (cNodeContent, NULL);
					pTextZone->bRefresh = TRUE;
						pTextZone->iTimer = pTextZone->iRefresh - 2 ;	// On triche sur le timer à la lecture du xml pour avoir un refresh						
				}
				else
				{
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "cmd") == 0 || xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "echo") == 0 || xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "internal") == 0)
					{
						pTextZone->bRefresh = TRUE;
						pTextZone->iRefresh = 0;
					}
				}				
				
				// On gère l'alignement à part pour lui imposer à 'left' si rien n'est renseigné (ou si mal renseigné)
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "align") == 0)
				{
					pTextZone->cAlign = xmlNodeGetContent (pXmlSubNode);					
					if (strcmp (pTextZone->cAlign, "left") == 0)
						pTextZone->cAlign = g_strdup_printf("left");
					else if (strcmp (pTextZone->cAlign, "center") == 0)
						pTextZone->cAlign = g_strdup_printf("center");
					else if (strcmp (pTextZone->cAlign, "right") == 0)
						pTextZone->cAlign = g_strdup_printf("right");
					else
						pTextZone->cAlign = g_strdup_printf("left");
				}
				else if (pTextZone->cAlign == NULL)
					pTextZone->cAlign = g_strdup_printf("left");		
				
				
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "font") == 0)
				{
					pTextZone->cFont = xmlNodeGetContent (pXmlSubNode);					
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "red") == 0)
				{
					pTextZone->fTextColor[0] = g_strtod (cNodeContent, NULL);					
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "green") == 0)
				{
					pTextZone->fTextColor[1] = g_strtod (cNodeContent, NULL);					
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "blue") == 0)
				{
					pTextZone->fTextColor[2] = g_strtod (cNodeContent, NULL);					
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "alpha") == 0)
				{
					pTextZone->fTextColor[3] = g_strtod (cNodeContent, NULL);					
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "txt") == 0)
				{
					pTextZone->cText = xmlNodeGetContent (pXmlSubNode);					
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "echo") == 0)
				{
					// On insère sh -c 'echo " AVANT la commande et "' APRES
					gchar *cXmlCommand;
					cXmlCommand = xmlNodeGetContent (pXmlSubNode);			
					
					GString *sTemp =  g_string_new  ("");	
					g_string_printf (sTemp, "sh -c 'echo \"%s\"'", cXmlCommand);							
					pTextZone->cCommand = g_strdup_printf("%s",sTemp->str) ;
					
					g_string_free (sTemp, TRUE);
					g_free (cXmlCommand);										
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "cmd") == 0)
				{
					pTextZone->cCommand = xmlNodeGetContent (pXmlSubNode);															
				}			
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "br") == 0)
				{
					pTextZone->bEndOfLine = TRUE;
					pTextZone->bNextNewLine = TRUE;														
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "nbr") == 0)
				{
					pTextZone->bEndOfLine = TRUE;
					pTextZone->bNextNewLine = FALSE;														
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "internal") == 0)
					pTextZone->cInternal = xmlNodeGetContent (pXmlSubNode);
				
				
				
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "lbar") == 0)
				{
					gchar *cTempo = xmlNodeGetContent (pXmlSubNode);

					// On récupère le 1er champ -> = commande interne
					g_strreverse (cTempo);
					cTempo = strrchr(cTempo, ';') ;
					ltrim( cTempo, ";" );
					g_strreverse (cTempo);
					pTextZone->cInternal = g_strdup_printf("%s", cTempo);
					
					// On récupère le 2ème champ -> = Largeur
					cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';') ;
					ltrim( cTempo, ";" );
					g_strreverse (cTempo);					
					cTempo = strchr(cTempo, ';') ;			
					ltrim( cTempo, ";" );
					g_strreverse (cTempo);										
					pTextZone->iWidth = atoi(cTempo); 
					
					// On récupère le 3ème champ -> = Hauteur
					cTempo = strrchr(xmlNodeGetContent (pXmlSubNode), ';') ;
					ltrim( cTempo, ";" );					
					pTextZone->iHeight = atoi(cTempo); 
					
					pTextZone->bLimitedBar = TRUE;					
				}
				
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "bar") == 0)
				{
					gchar *cTempo = xmlNodeGetContent (pXmlSubNode);

					// On récupère le 1er champ -> = commande interne
					g_strreverse (cTempo);
					cTempo = strrchr(cTempo, ';') ;
					ltrim( cTempo, ";" );
					g_strreverse (cTempo);
					pTextZone->cInternal = g_strdup_printf("%s", cTempo);
										
					// On récupère le 3ème champ -> = Hauteur
					cTempo = strrchr(xmlNodeGetContent (pXmlSubNode), ';') ;
					ltrim( cTempo, ";" );					
					pTextZone->iHeight = atoi(cTempo); 
					
					pTextZone->bBar = TRUE;					
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "img") == 0)
				{
					pTextZone->cImgPath = xmlNodeGetContent (pXmlSubNode);
					pTextZone->bImgDraw=FALSE;		
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "imgsize") == 0)
				{
					pTextZone->iImgSize = g_strtod (cNodeContent, NULL);				
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "imgsizeW") == 0)
				{
					pTextZone->iWidth= g_strtod (cNodeContent, NULL);				
				}
				else if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "imgsizeH") == 0)
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
