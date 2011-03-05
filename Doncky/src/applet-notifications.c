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

//\________________ Add your name in the copyright file (and / or modify your name here)

#include <stdlib.h>
#include <string.h>
#include "applet-draw.h"
#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-xml.h"


static void _new_xml_to_conf (CairoDockModuleInstance *myApplet, gchar *cReceivedData)
{
	// if (cReceivedData && strncmp (cReceivedData, "http://", 7) == 0 && g_str_has_suffix (cReceivedData, ".tar.gz") && (g_strstr_len (cReceivedData, -1, "glxdock") || g_strstr_len (cReceivedData, -1, "glx-dock")))
	//~ if (g_strstr_len (cReceivedData, -1, ".xml") != NULL)  // On verifie que l'element glisser/copier fini bien par .xml
	if (cReceivedData && (strncmp (cReceivedData, "http://", 7) == 0 && g_str_has_suffix (cReceivedData, ".tar.gz")) \
			|| (strncmp (cReceivedData, "http://", 7) == 0 && g_str_has_suffix (cReceivedData, ".xml")) \
			|| (strncmp (cReceivedData, "file://", 7) == 0 && g_str_has_suffix (cReceivedData, ".xml")))
	{
		if (strncmp (cReceivedData, "file://", 7) == 0)
		{
			ltrim( cReceivedData, "file:///" );
			cReceivedData = g_strdup_printf("/%s", cReceivedData);
		}
		
		cd_debug ("DONCKY-debug : \"%s\" was dropped", cReceivedData);
		
		cd_debug ("DONCKY-debug : This seems to be a valid XML File -> Let's continue...");
		// on definit la nouvelle URL en conf.
		g_free (myConfig.cXmlFilePath);
		myConfig.cXmlFilePath = g_strdup (cReceivedData);
		cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
			G_TYPE_STRING,
			"Configuration",
			"xml_filepath",
			myConfig.cXmlFilePath,
			G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
		
		cd_doncky_free_item_list (myApplet);
		cd_doncky_readxml (myApplet);
		
		// 2 times to allow xml files to modify the conf :		
		cd_doncky_free_item_list (myApplet);
		cd_doncky_readxml (myApplet);
	}
	else
	{
		cd_debug ("DONCKY-debug : It doesn't seem to be a valid XML.");	
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog_with_icon (D_("It doesn't seem to be a valid XML file."),
			myIcon,
			myContainer,
			3000, // Suffisant 
			MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);		
	}
}


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
//~ CD_APPLET_ON_CLICK_BEGIN
	//~ 
//~ CD_APPLET_ON_CLICK_END


//~ CD_APPLET_ON_DOUBLE_CLICK_BEGIN
	//~ 
//~ CD_APPLET_ON_DOUBLE_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


//~ CD_APPLET_ON_SCROLL_BEGIN
//~ 
//~ CD_APPLET_ON_SCROLL_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_debug ("DONCKY-debug : \"%s\" was dropped", CD_APPLET_RECEIVED_DATA);
	_new_xml_to_conf (myApplet, g_strdup_printf("%s", CD_APPLET_RECEIVED_DATA));
	
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

	
		
	int i;
	for (pXmlNode = pXmlMainNode->children, i = 0; pXmlNode != NULL; pXmlNode = pXmlNode->next, i ++)
	{
				
		if (xmlStrcmp (pXmlNode->name, (const xmlChar *) "appearance") == 0)
		{
			gchar *cTempo = NULL;
			
			xmlNodePtr pXmlSubNode;			
			for (pXmlSubNode = pXmlNode->children; pXmlSubNode != NULL; pXmlSubNode = pXmlSubNode->next)
			{				
				cNodeContent = xmlNodeGetContent (pXmlSubNode);
				
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "deskletSize") == 0)
				{
					cTempo = xmlNodeGetContent (pXmlSubNode);
					cd_debug ("DONCKY-debug : size = %s", cTempo);
					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "size", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "defaultFont") == 0)
				{
					cTempo = xmlNodeGetContent (pXmlSubNode);
					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "default_font", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					myConfig.cDefaultFont = g_strdup_printf("%s", cTempo);
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "defaultTextColor") == 0)
				{
					gchar *cStringInConfig = NULL;

					// On récupère le 1er champ -> red
					cTempo = xmlNodeGetContent (pXmlSubNode);
					// Le atof ne traitera que le 1er champ
					myConfig.fDefaultTextColor[0] = atof(cTempo) / 255;

					// On récupère le 2ème champ -> = green
					cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" ); // On ne coupe que le 1er ; et on garde le reste qui ne sera pas traité par le atof
					myConfig.fDefaultTextColor[1] = atof(cTempo) / 255;
					
					// On récupère le 3ème champ -> = blue
					cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" );
					cTempo = strchr(cTempo, ';'); 
					ltrim( cTempo, ";" ); // On ne coupe que le 1er ; et on garde le reste qui ne sera pas traité par le atof
					myConfig.fDefaultTextColor[2] = atof(cTempo) / 255;

					// On récupère le dernier champ -> alpha
					cTempo = strrchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" );
					myConfig.fDefaultTextColor[3] = atof(cTempo) / 255;
					
					// Remplacement des virgules par des points pour l'écriture dans la config					
					cTempo = g_strdup_printf("%f", myConfig.fDefaultTextColor[0]);
					cStringInConfig = g_strdup_printf("%s", g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fDefaultTextColor[1]);
					cStringInConfig = g_strdup_printf("%s;%s", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fDefaultTextColor[2]);
					cStringInConfig = g_strdup_printf("%s;%s", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fDefaultTextColor[3]);
					cStringInConfig = g_strdup_printf("%s;%s;", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cd_debug ("DONCKY-debug : default_text_color=%s", cStringInConfig); 

					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "default_text_color", cStringInConfig, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					g_free(cStringInConfig);

					// Mise à jour des PrevTextColor
					myData.fPrevTextColor[0] = myConfig.fDefaultTextColor[0];
					myData.fPrevTextColor[1] = myConfig.fDefaultTextColor[1];
					myData.fPrevTextColor[2] = myConfig.fDefaultTextColor[2];
					myData.fPrevTextColor[3] = myConfig.fDefaultTextColor[3];
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "textMargin") == 0)
				{
					cTempo = xmlNodeGetContent (pXmlSubNode);
					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "text_margin", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					myConfig.iTextMargin = atoi(cTempo);
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "spaceBetweenLines") == 0)
				{
					cTempo = xmlNodeGetContent (pXmlSubNode);
					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "space_between_lines", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					myConfig.iSpaceBetweenLines = atoi(cTempo);
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "displayBackground") == 0)
				{
					cTempo = xmlNodeGetContent (pXmlSubNode);
					if (strcmp (cTempo, "0") == 0)
					{
						cTempo = g_strdup_printf("false");
						myConfig.bDisplayBackground = FALSE;
					}
					else
					{
						cTempo = g_strdup_printf("true");
						myConfig.bDisplayBackground = TRUE;
					}
					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "display_background", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "backgroundColor1") == 0)
				{
					gchar *cStringInConfig = NULL;

					// On récupère le 1er champ -> red
					cTempo = xmlNodeGetContent (pXmlSubNode);
					// Le atof ne traitera que le 1er champ
					myConfig.fBackgroundColor1[0] = atof(cTempo) / 255;

					// On récupère le 2ème champ -> = green
					cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" ); // On ne coupe que le 1er ; et on garde le reste qui ne sera pas traité par le atof
					myConfig.fBackgroundColor1[1] = atof(cTempo) / 255;
					
					// On récupère le 3ème champ -> = blue
					cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" );
					cTempo = strchr(cTempo, ';'); 
					ltrim( cTempo, ";" ); // On ne coupe que le 1er ; et on garde le reste qui ne sera pas traité par le atof
					myConfig.fBackgroundColor1[2] = atof(cTempo) / 255;

					// On récupère le dernier champ -> alpha
					cTempo = strrchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" );
					myConfig.fBackgroundColor1[3] = atof(cTempo) / 255;
					
					// Remplacement des virgules par des points pour l'écriture dans la config					
					cTempo = g_strdup_printf("%f", myConfig.fBackgroundColor1[0]);
					cStringInConfig = g_strdup_printf("%s", g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fBackgroundColor1[1]);
					cStringInConfig = g_strdup_printf("%s;%s", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fBackgroundColor1[2]);
					cStringInConfig = g_strdup_printf("%s;%s", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fBackgroundColor1[3]);
					cStringInConfig = g_strdup_printf("%s;%s;", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cd_debug ("DONCKY-debug : background_color1=%s", cStringInConfig); 

					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "background_color1", cStringInConfig, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config

					g_free(cStringInConfig);
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "backgroundColor2") == 0)
				{
					gchar *cStringInConfig = NULL;

					// On récupère le 1er champ -> red
					cTempo = xmlNodeGetContent (pXmlSubNode);
					// Le atof ne traitera que le 1er champ
					myConfig.fBackgroundColor2[0] = atof(cTempo) / 255;

					// On récupère le 2ème champ -> = green
					cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" ); // On ne coupe que le 1er ; et on garde le reste qui ne sera pas traité par le atof
					myConfig.fBackgroundColor2[1] = atof(cTempo) / 255;
					
					// On récupère le 3ème champ -> = blue
					cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" );
					cTempo = strchr(cTempo, ';'); 
					ltrim( cTempo, ";" ); // On ne coupe que le 1er ; et on garde le reste qui ne sera pas traité par le atof
					myConfig.fBackgroundColor2[2] = atof(cTempo) / 255;

					// On récupère le dernier champ -> alpha
					cTempo = strrchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" );
					myConfig.fBackgroundColor2[3] = atof(cTempo) / 255;
					
					// Remplacement des virgules par des points pour l'écriture dans la config					
					cTempo = g_strdup_printf("%f", myConfig.fBackgroundColor2[0]);
					cStringInConfig = g_strdup_printf("%s", g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fBackgroundColor2[1]);
					cStringInConfig = g_strdup_printf("%s;%s", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fBackgroundColor2[2]);
					cStringInConfig = g_strdup_printf("%s;%s", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fBackgroundColor2[3]);
					cStringInConfig = g_strdup_printf("%s;%s;", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cd_debug ("DONCKY-debug : background_color2=%s", cStringInConfig); 

					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "background_color2", cStringInConfig, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config

					g_free(cStringInConfig);
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "backgroundRadius") == 0)
				{
					cTempo = xmlNodeGetContent (pXmlSubNode);
					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "background_radius", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					myConfig.iBackgroundRadius = atoi(cTempo);
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "borderThickness") == 0)
				{
					cTempo = xmlNodeGetContent (pXmlSubNode);
					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "border_thickness", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					myConfig.iBorderThickness = atoi(cTempo);
				}
				if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "borderColor") == 0)
				{
					gchar *cStringInConfig = NULL;

					// On récupère le 1er champ -> red
					cTempo = xmlNodeGetContent (pXmlSubNode);
					// Le atof ne traitera que le 1er champ
					myConfig.fBorderColor[0] = atof(cTempo) / 255;

					// On récupère le 2ème champ -> = green
					cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" ); // On ne coupe que le 1er ; et on garde le reste qui ne sera pas traité par le atof
					myConfig.fBorderColor[1] = atof(cTempo) / 255;
					
					// On récupère le 3ème champ -> = blue
					cTempo = strchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" );
					cTempo = strchr(cTempo, ';'); 
					ltrim( cTempo, ";" ); // On ne coupe que le 1er ; et on garde le reste qui ne sera pas traité par le atof
					myConfig.fBorderColor[2] = atof(cTempo) / 255;

					// On récupère le dernier champ -> alpha
					cTempo = strrchr(xmlNodeGetContent (pXmlSubNode), ';');
					ltrim( cTempo, ";" );
					myConfig.fBorderColor[3] = atof(cTempo) / 255;
					
					// Remplacement des virgules par des points pour l'écriture dans la config					
					cTempo = g_strdup_printf("%f", myConfig.fBorderColor[0]);
					cStringInConfig = g_strdup_printf("%s", g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fBorderColor[1]);
					cStringInConfig = g_strdup_printf("%s;%s", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fBorderColor[2]);
					cStringInConfig = g_strdup_printf("%s;%s", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cTempo = g_strdup_printf("%f", myConfig.fBorderColor[3]);
					cStringInConfig = g_strdup_printf("%s;%s;", cStringInConfig, g_str_replace (cTempo, ",", "."));
					cd_debug ("DONCKY-debug : border_color=%s", cStringInConfig); 

					cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "border_color", cStringInConfig, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config

					g_free(cStringInConfig);
				}
				xmlFree (cNodeContent);
			}
			g_free (cTempo);
			
			gchar *cCommand = g_strdup_printf ("dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.ReloadModule string:Doncky");
			gchar *cResult = cairo_dock_launch_command_sync (cCommand);
			g_free (cCommand);
			// redessin.
			//~ cd_applet_update_my_icon (myApplet);
		}				
	}	
	cairo_dock_close_xml_file (pXmlFile);
	
CD_APPLET_ON_DROP_DATA_END



CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cd_doncky_free_item_list (myApplet);
	cd_doncky_readxml (myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END
