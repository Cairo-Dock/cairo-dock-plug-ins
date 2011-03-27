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


//~ #include <glib.h>
#include <glib/gstdio.h>

gboolean _check_size_is_constant (CairoDockModuleInstance *myApplet, const gchar *cFilePath)
{
	int iSize = cairo_dock_get_file_size (cFilePath);
	gchar *cCommand = g_strdup_printf ("ping 127.0.0.1 -i 0.2 -c 2"); // On fait un temps d'arret de 200ms
	cairo_dock_launch_command (cCommand);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	g_free (cResult);
	gboolean bConstantSize = (iSize != 0 && iSize == myData.iCurrentFileSize);
	myData.iCurrentFileSize = iSize;
	
	return bConstantSize;
}

gboolean _new_xml_to_conf (CairoDockModuleInstance *myApplet, gchar *cReceivedData)
{
	gboolean bContinue = FALSE;
	
	if (cReceivedData && (strncmp (cReceivedData, "file://", 7) == 0 && g_str_has_suffix (cReceivedData, ".xml")) \
			|| (strncmp (cReceivedData, "file://", 7) == 0 && g_str_has_suffix (cReceivedData, ".tar.gz")) \
			|| (strncmp (cReceivedData, "http://", 7) == 0 && g_str_has_suffix (cReceivedData, ".xml")) \
			|| (strncmp (cReceivedData, "http://", 7) == 0 && g_str_has_suffix (cReceivedData, ".tar.gz")))
	{
		
		
		if (strncmp (cReceivedData, "file://", 7) == 0 && g_str_has_suffix (cReceivedData, ".xml")) // On laisse le fichier où il est et on ne crée pas de thème dans ~/.config/cairo-dock/doncky/
		{
			cd_debug ("DONCKY-debug : local xml file -> Use it without any copy.");
			ltrim( cReceivedData, "file:///" );
			cReceivedData = g_strdup_printf("/%s", cReceivedData);
			bContinue = TRUE;
		}
		else // On crée un thème dans ~/.config/cairo-dock/doncky/
		{
			gchar *cTmpFileName = g_strdup_printf("%s", cReceivedData);
			
			if (strncmp (cReceivedData, "file://", 7) == 0)
			{
				ltrim(cTmpFileName, "file:///");
				cReceivedData = g_strdup_printf("%s", cTmpFileName);
			}
			else if (strncmp (cReceivedData, "http://", 7) == 0)
				ltrim(cTmpFileName,"http://");
					
			// On récupère le 1er champ -> nom du fichier
			g_strreverse (cTmpFileName);			
			cTmpFileName = g_str_position (cTmpFileName, 1, '/');
			g_strreverse (cTmpFileName);
			
			
			// Récupération du nom du fichier sans l'extension
			gchar *cTmpThemeName = g_strdup_printf("%s", cTmpFileName);
			if (g_str_has_suffix(cReceivedData,".xml"))
				rtrim(cTmpThemeName,".xml");
			else if (g_str_has_suffix(cReceivedData,".tar.gz"))
			{
				cTmpThemeName = g_str_position (cTmpThemeName, 1, '.');
			}
			
			cd_debug ("DONCKY-debug : Theme name : %s", cTmpThemeName);
			
			// on cree le repertoire pour le theme.
			gchar *cDonckyThemesPath = g_strdup_printf ("%s/doncky", g_cCairoDockDataDir);
			gchar *cThemePath = g_strdup_printf ("%s/%s", cDonckyThemesPath, cTmpThemeName);
			
			if (! g_file_test (cThemePath, G_FILE_TEST_EXISTS))
			{
				cd_debug ("DONCKY-debug : the folder '%s' doesn't exist -> We create it", cThemePath);
				
				if (! g_file_test (cDonckyThemesPath, G_FILE_TEST_EXISTS))
				{
					cd_debug ("DONCKY-debug : the folder '%s' doesn't exist -> We create it", cDonckyThemesPath);
					if (g_mkdir (cDonckyThemesPath, 7*8*8+7*8+0) != 0)
					{
						cd_warning ("couldn't create directory '%s' !\nNo read history will be available.", cDonckyThemesPath);
						bContinue = FALSE;
					}
				}
				if (g_mkdir (cThemePath, 7*8*8+7*8+0) != 0)
				{
					cd_warning ("couldn't create directory '%s' !\nNo read history will be available.", cThemePath);
					bContinue = FALSE;
				}
				else
					bContinue = TRUE;
			}
			else
			{
				cd_debug ("DONCKY-debug : the folder '%s' exists -> Asking what to do ...", cThemePath);
				int iAnswer = GTK_RESPONSE_YES;
				iAnswer = cairo_dock_ask_question_and_wait ("A theme with the same name already exists in ~/.config/cairo-dock/doncky. Do you want to overwrite it ?", myIcon, myContainer);
				if (iAnswer == GTK_RESPONSE_YES)
				{
					gchar *cCommand = g_strdup_printf ("cd \"%s\" && rm *.*", cThemePath);
					//~ cairo_dock_launch_command (cCommand);
					gchar *cResult = cairo_dock_launch_command_sync (cCommand);
					g_free (cCommand);
					bContinue = TRUE;
				}
				else
				{
					bContinue = FALSE;
				}					
				
			}
			
			if (bContinue)
			{
				if (strncmp (cReceivedData, "http://", 7) == 0)
				{
					gchar *cCommand = g_strdup_printf ("wget \"%s\" -O \"%s/%s\" -t 3 -T 4 30 /dev/null 2>&1", cReceivedData, cThemePath, cTmpFileName);
					cd_debug ("DONCKY-debug : Downloading the file ...");
					cairo_dock_launch_command (cCommand);
					g_free (cCommand);												
				}
				else // C'est donc un fichier local
				{
					gchar *cCommand = g_strdup_printf ("cp \"/%s\" \"%s/%s\"", cReceivedData, cThemePath, cTmpFileName);
					cairo_dock_launch_command (cCommand);
					g_free (cCommand);
				}
				
				cReceivedData = g_strdup_printf("%s/%s", cThemePath, cTmpFileName);
				
				cd_debug ("DONCKY-debug : Waiting to complete \"%s\"...", cReceivedData);
				do
				{
					// waiting for the download to be completed
					cd_debug ("DONCKY-debug : Waiting to complete...");
				}while (!_check_size_is_constant (myApplet, cReceivedData));
				
				cd_debug ("DONCKY-debug : \"%s\" is ready (Downloaded size : %i octets)", cReceivedData, myData.iCurrentFileSize);
				
				if (g_str_has_suffix(cReceivedData,".tar.gz"))
				{
					gchar *cCommand = g_strdup_printf ("cd \"%s\" && tar -xzvf \"%s\"",cThemePath, cTmpFileName);
					cairo_dock_launch_command (cCommand);
					g_free (cCommand);
					// On re-définit le chemin du xml
					cTmpFileName = g_str_position (cTmpFileName, 1, '.');
					cReceivedData = g_strdup_printf("%s/%s.xml", cThemePath, cTmpFileName);
				}
				
			}
			g_free (cDonckyThemesPath);
			g_free (cThemePath);
			g_free (cTmpFileName);
			g_free (cTmpThemeName);
		}
		
		if (bContinue)
		{
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
			return TRUE;
		}
		else
			return FALSE;
		
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
		return FALSE;
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
	
	if (_new_xml_to_conf (myApplet, g_strdup_printf("%s", CD_APPLET_RECEIVED_DATA)))
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
					
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "icon_size") == 0)
					{
						cd_debug ("DONCKY-debug : icon_size = %s", xmlNodeGetContent (pXmlSubNode));
						cTempo = g_strdup_printf("%s;",xmlNodeGetContent (pXmlSubNode));
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Icon", "icon size", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "deskletSize") == 0)
					{
						cd_debug ("DONCKY-debug : desklet size = %s", xmlNodeGetContent (pXmlSubNode));
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "size", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "decorations") == 0)
					{
						cd_debug ("DONCKY-debug : decorations = %s", xmlNodeGetContent (pXmlSubNode));
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "decorations", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "bg_desklet") == 0)
					{
						cTempo = _Get_FilePath (myApplet, xmlNodeGetContent (pXmlSubNode));
						cd_debug ("DONCKY-debug : bg desklet = %s", cTempo);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "bg desklet", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "bg_alpha") == 0)
					{
						cd_debug ("DONCKY-debug : bg_alpha = %s", xmlNodeGetContent (pXmlSubNode));
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "bg alpha", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "left_offset") == 0)
					{
						cd_debug ("DONCKY-debug : left_offset = %s", xmlNodeGetContent (pXmlSubNode));
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "left offset", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "top_offset") == 0)
					{
						cd_debug ("DONCKY-debug : top_offset = %s", xmlNodeGetContent (pXmlSubNode));
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "top offset", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "right_offset") == 0)
					{
						cd_debug ("DONCKY-debug : right_offset = %s", xmlNodeGetContent (pXmlSubNode));
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "right offset", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "bottom_offset") == 0)
					{
						cd_debug ("DONCKY-debug : bottom_offset = %s", xmlNodeGetContent (pXmlSubNode));
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "bottom offset", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "fg_desklet") == 0)
					{
						cTempo = _Get_FilePath (myApplet, xmlNodeGetContent (pXmlSubNode));
						cd_debug ("DONCKY-debug : fg desklet = %s", cTempo);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "fg desklet", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "fg_alpha") == 0)
					{
						cd_debug ("DONCKY-debug : fg_alpha = %s", xmlNodeGetContent (pXmlSubNode));
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "fg alpha", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "defaultFont") == 0)
					{
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "default_font", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						myConfig.cDefaultFont = g_strdup_printf("%s", xmlNodeGetContent (pXmlSubNode));
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "defaultTextColor") == 0)
					{
						gchar *cStringInConfig = NULL;
						
						// On récupère le 1er champ -> red
						myConfig.fDefaultTextColor[0] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 1, ';')) / 255;
						// On récupère le 2ème champ -> = green
						myConfig.fDefaultTextColor[1] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 2, ';')) / 255;
						// On récupère le 3ème champ -> = blue
						myConfig.fDefaultTextColor[2] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 3, ';')) / 255;
						// On récupère le dernier champ -> alpha
						myConfig.fDefaultTextColor[3] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 4, ';')) / 255;
	
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
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "text_margin", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						myConfig.iTextMargin = atoi(xmlNodeGetContent (pXmlSubNode));
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "spaceBetweenLines") == 0)
					{
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "space_between_lines", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						myConfig.iSpaceBetweenLines = atoi(xmlNodeGetContent (pXmlSubNode));
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
						myConfig.fBackgroundColor1[0] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 1, ';')) / 255;
						// On récupère le 2ème champ -> = green
						myConfig.fBackgroundColor1[1] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 2, ';')) / 255;
						// On récupère le 3ème champ -> = blue
						myConfig.fBackgroundColor1[2] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 3, ';')) / 255;
						// On récupère le dernier champ -> alpha
						myConfig.fBackgroundColor1[3] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 4, ';')) / 255;
						
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
						myConfig.fBackgroundColor2[0] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 1, ';')) / 255;
						// On récupère le 2ème champ -> = green
						myConfig.fBackgroundColor2[1] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 2, ';')) / 255;
						// On récupère le 3ème champ -> = blue
						myConfig.fBackgroundColor2[2] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 3, ';')) / 255;
						// On récupère le dernier champ -> alpha
						myConfig.fBackgroundColor2[3] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 4, ';')) / 255;
						
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
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "background_radius", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						myConfig.iBackgroundRadius = atoi(xmlNodeGetContent (pXmlSubNode));
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "borderThickness") == 0)
					{
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "border_thickness", xmlNodeGetContent (pXmlSubNode), G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						myConfig.iBorderThickness = atoi(xmlNodeGetContent (pXmlSubNode));
					}
					if (xmlStrcmp (pXmlSubNode->name, (const xmlChar *) "borderColor") == 0)
					{
						gchar *cStringInConfig = NULL;
						
						// On récupère le 1er champ -> red
						myConfig.fBorderColor[0] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 1, ';')) / 255;
						// On récupère le 2ème champ -> = green
						myConfig.fBorderColor[1] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 2, ';')) / 255;
						// On récupère le 3ème champ -> = blue
						myConfig.fBorderColor[2] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 3, ';')) / 255;
						// On récupère le dernier champ -> alpha
						myConfig.fBorderColor[3] = atof(g_str_position (xmlNodeGetContent (pXmlSubNode), 4, ';')) / 255;
	
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
				
				cairo_dock_reload_module_instance (myApplet, TRUE); // TRUE <=> read conf file
			}				
		}	
		cairo_dock_close_xml_file (pXmlFile);
	}
	
CD_APPLET_ON_DROP_DATA_END



CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cd_doncky_free_item_list (myApplet);
	cd_doncky_readxml (myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END
