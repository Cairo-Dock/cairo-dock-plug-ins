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

gboolean _check_size_is_constant (GldiModuleInstance *myApplet, const gchar *cFilePath)
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

gboolean _new_xml_to_conf (GldiModuleInstance *myApplet, gchar *cReceivedData)
{
	gboolean bContinue = FALSE;
	
	if (cReceivedData && ((strncmp (cReceivedData, "file://", 7) == 0 && g_str_has_suffix (cReceivedData, ".xml")) \
			|| (strncmp (cReceivedData, "file://", 7) == 0 && g_str_has_suffix (cReceivedData, ".tar.gz")) \
			|| (strncmp (cReceivedData, "http://", 7) == 0 && g_str_has_suffix (cReceivedData, ".xml")) \
			|| (strncmp (cReceivedData, "http://", 7) == 0 && g_str_has_suffix (cReceivedData, ".tar.gz"))))
	{
		
		
		if (strncmp (cReceivedData, "file://", 7) == 0 && g_str_has_suffix (cReceivedData, ".xml")) // On laisse le fichier où il est et on ne crée pas de thème dans ~/.config/cairo-dock/doncky/
		{
			cd_debug ("DONCKY-debug : local xml file -> Use it without any copy.");
			// ltrim( cReceivedData, "file:///" );
			cReceivedData = g_filename_from_uri (cReceivedData, NULL, NULL);// FREE // g_strdup_printf("/%s", cReceivedData);
			bContinue = TRUE;
		}
		else // On crée un thème dans ~/.config/cairo-dock/doncky/
		{
			if (strncmp (cReceivedData, "file://", 7) == 0)
			{
				cReceivedData = g_filename_from_uri (cReceivedData, NULL, NULL); // FREE
			}

			// On récupère le 1er champ -> nom du fichier
			/*g_strreverse (cTmpFileName);			
			cTmpFileName = g_str_position (cTmpFileName, 1, '/');
			g_strreverse (cTmpFileName);*/
			gchar *cTmpFileName = g_path_get_basename (cReceivedData);
			
			
			
			// Récupération du nom du fichier sans l'extension
			gchar *cTmpThemeName = g_strdup (cTmpFileName);
			if (g_str_has_suffix(cReceivedData,".xml"))
				rtrim(cTmpThemeName,".xml");
			else if (g_str_has_suffix(cReceivedData,".tar.gz"))
			{
				rtrim (cTmpThemeName, ".tar.gz");
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
				cd_debug ("DONCKY-debug : the folder '%s' exists -> give it a new name ...", cThemePath);
				gchar *cNewPath=NULL;
				int i=2;
				do
				{
					cNewPath = g_strdup_printf ("%s-%d", cThemePath, i);
					i ++;
				}
				while (g_file_test (cNewPath, G_FILE_TEST_EXISTS));
				g_free (cThemePath);
				cThemePath = cNewPath;
				bContinue = TRUE;
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

				g_free (cReceivedData);
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
					rtrim (cTmpFileName, ".tar.gz");
					g_free (cReceivedData);
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
		gldi_dialogs_remove_on_icon (myIcon);
		gldi_dialog_show_temporary_with_icon (D_("It doesn't seem to be a valid XML file."),
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
/* CD_APPLET_ON_BUILD_MENU_BEGIN
	
CD_APPLET_ON_BUILD_MENU_END*/


//~ CD_APPLET_ON_SCROLL_BEGIN
//~ 
//~ CD_APPLET_ON_SCROLL_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_debug ("DONCKY-debug : \"%s\" was dropped", CD_APPLET_RECEIVED_DATA);
	
	if (_new_xml_to_conf (myApplet, g_strdup (CD_APPLET_RECEIVED_DATA)))
	{	
		// On va lire le contenu de myConfig.cXmlFilePath	
		cd_debug ("Doncky-debug : ---------------------->  myConfig.cXmlFilePath = \"%s\"",myConfig.cXmlFilePath);
			
		g_return_val_if_fail (myConfig.cXmlFilePath != NULL, FALSE);
		xmlInitParser ();
		xmlDocPtr pXmlFile;
		xmlNodePtr pXmlMainNode;
		
		pXmlFile = cairo_dock_open_xml_file (myConfig.cXmlFilePath, "doncky", &pXmlMainNode, NULL);
		
		g_return_val_if_fail (pXmlFile != NULL && pXmlMainNode != NULL, FALSE);
			
		xmlNodePtr pXmlNode;
		
		myData.cPrevFont = g_strdup (myConfig.cDefaultFont);
		myData.cPrevAlignWidth = g_strdup_printf("left");
		myData.cPrevAlignHeight = g_strdup_printf("middle");
	
		int i;
		for (pXmlNode = pXmlMainNode->children, i = 0; pXmlNode != NULL; pXmlNode = pXmlNode->next, i ++)
		{
					
			if (xmlStrcmp (pXmlNode->name, BAD_CAST "appearance") == 0)
			{
				gchar *cTempo = NULL;
				gchar *cNodeContent = NULL;
				
				xmlNodePtr pXmlSubNode;
				for (pXmlSubNode = pXmlNode->children; pXmlSubNode != NULL; pXmlSubNode = pXmlSubNode->next)
				{
					cNodeContent = (gchar *) xmlNodeGetContent (pXmlSubNode);
					if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "icon_size") == 0)
					{
						cd_debug ("DONCKY-debug : icon_size = %s", cNodeContent);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Icon", "icon size", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "deskletSize") == 0)
					{
						cd_debug ("DONCKY-debug : desklet size = %s", cNodeContent);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "size", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "decorations") == 0)
					{
						cd_debug ("DONCKY-debug : decorations = %s", cNodeContent);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "decorations", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "bg_desklet") == 0)
					{
						cTempo = _Get_FilePath (myApplet, cNodeContent);
						cd_debug ("DONCKY-debug : bg desklet = %s", cTempo);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "bg desklet", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						g_free (cTempo);
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "bg_alpha") == 0)
					{
						cd_debug ("DONCKY-debug : bg_alpha = %s", cNodeContent);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "bg alpha", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "left_offset") == 0)
					{
						cd_debug ("DONCKY-debug : left_offset = %s", cNodeContent);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "left offset", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "top_offset") == 0)
					{
						cd_debug ("DONCKY-debug : top_offset = %s", cNodeContent);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "top offset", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "right_offset") == 0)
					{
						cd_debug ("DONCKY-debug : right_offset = %s", cNodeContent);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "right offset", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "bottom_offset") == 0)
					{
						cd_debug ("DONCKY-debug : bottom_offset = %s", cNodeContent);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "bottom offset", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "fg_desklet") == 0)
					{
						cTempo = _Get_FilePath (myApplet, cNodeContent);
						cd_debug ("DONCKY-debug : fg desklet = %s", cTempo);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "fg desklet", cTempo, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						g_free (cTempo);
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "fg_alpha") == 0)
					{
						cd_debug ("DONCKY-debug : fg_alpha = %s", cNodeContent);
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Desklet", "fg alpha", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "defaultFont") == 0)
					{
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "default_font", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						g_free (myConfig.cDefaultFont);
						myConfig.cDefaultFont = g_strdup (cNodeContent);
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "defaultTextColor") == 0)
					{
						cd_doncky_get_color_from_xml (cNodeContent, myConfig.fDefaultTextColor);
	
						// Remplacement des virgules par des points pour l'écriture dans la config
						
						cd_doncky_export_color_to_conf (myConfig.fDefaultTextColor, "Appearance", "default_text_color", myApplet);
	
						// Mise à jour des PrevTextColor
						myData.fPrevTextColor[0] = myConfig.fDefaultTextColor[0];
						myData.fPrevTextColor[1] = myConfig.fDefaultTextColor[1];
						myData.fPrevTextColor[2] = myConfig.fDefaultTextColor[2];
						myData.fPrevTextColor[3] = myConfig.fDefaultTextColor[3];
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "textMargin") == 0)
					{
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "text_margin", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						myConfig.iTextMargin = atoi(cNodeContent);
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "spaceBetweenLines") == 0)
					{
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "space_between_lines", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						myConfig.iSpaceBetweenLines = atoi(cNodeContent);
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "displayBackground") == 0)
					{
						if (strcmp (cNodeContent, "0") == 0)
						{
							cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "display_background", "false", G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
							myConfig.bDisplayBackground = FALSE;
						}
						else
						{
							cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "display_background", "true", G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
							myConfig.bDisplayBackground = TRUE;
						}
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "backgroundColor1") == 0)
					{
						cd_doncky_get_color_from_xml (cNodeContent, myConfig.fBackgroundColor1);

						cd_doncky_export_color_to_conf (myConfig.fBackgroundColor1, "Appearance", "background_color1", myApplet);
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "backgroundColor2") == 0)
					{
						cd_doncky_get_color_from_xml (cNodeContent, myConfig.fBackgroundColor2);

						cd_doncky_export_color_to_conf (myConfig.fBackgroundColor2, "Appearance", "background_color2", myApplet);
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "backgroundRadius") == 0)
					{
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "background_radius", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						myConfig.iBackgroundRadius = atoi(cNodeContent);
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "borderThickness") == 0)
					{
						cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE, G_TYPE_STRING, "Appearance", "border_thickness", cNodeContent, G_TYPE_INVALID);  // On l'ecrit dans le fichier de config
						myConfig.iBorderThickness = atoi(cNodeContent);
					}
					else if (xmlStrcmp (pXmlSubNode->name, BAD_CAST "borderColor") == 0)
					{
						cd_doncky_get_color_from_xml (cNodeContent, myConfig.fBorderColor);

						cd_doncky_export_color_to_conf (myConfig.fBorderColor, "Appearance", "border_color", myApplet);
					}
					g_free (cTempo);
					g_free (cNodeContent);
				}
				
				gldi_module_instance_reload (myApplet, TRUE); // TRUE <=> read conf file
			}				
		}	
		cairo_dock_close_xml_file (pXmlFile);
	}
	
CD_APPLET_ON_DROP_DATA_END



CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cd_doncky_free_item_list (myApplet);
	cd_doncky_readxml (myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END
