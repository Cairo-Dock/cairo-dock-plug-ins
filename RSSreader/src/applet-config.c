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

#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-draw.h"

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN

	double couleur[4] = {0., 0., 0.5, 1.};
	
	// Section Configuration
	myConfig.cUrl = CD_CONFIG_GET_STRING ("Configuration", "url_rss_feed");
	myConfig.iLines = CD_CONFIG_GET_INTEGER ("Configuration", "lines_rss_feed");
	myConfig.iTitleNum = CD_CONFIG_GET_INTEGER ("Configuration", "title_num_rss_feed");	
	myConfig.cName = CD_CONFIG_GET_STRING ("Configuration", "name_rss_feed");

	// Réglages pour le titre
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "title_color", myConfig.fTitleTextColor, couleur);
	myConfig.cTitleFont = CD_CONFIG_GET_STRING ("Configuration", "title_font");
	myData.cTitleFontSize = g_strdup_printf ("%s", myConfig.cTitleFont);
	myData.cTitleFontSize = strrchr(myData.cTitleFontSize, ' ');  // On enlève tout ce qui se trouve avant le dernier 'espace'
	myData.cTitleFontSize = ltrim( myData.cTitleFontSize, " " );  // On supprime l'espace -> Il ne nous reste QUE la taille ;-)	
	if (g_strstr_len (myConfig.cTitleFont, -1, "Bold") != NULL)
		myData.bTitleFontIsBold = TRUE;
    else
		myData.bTitleFontIsBold = FALSE;    	
    if (g_strstr_len (myConfig.cTitleFont, -1, "Italic") != NULL)
		myData.bTitleFontIsItalic = TRUE;
    else
		myData.bTitleFontIsItalic = FALSE;	
	// On ne veut garder QUE le nom et pas la taille dans la variable... Ex: 'Sans' au lieu de 'Sans 12' ;-)
	g_strreverse (myConfig.cTitleFont);
	myConfig.cTitleFont = strchr(myConfig.cTitleFont, ' ');
	ltrim( myConfig.cTitleFont, " " );
	g_strreverse (myConfig.cTitleFont);
	
	// Réglages pour le texte du flux
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Configuration", "text_color", myConfig.fTextColor, couleur);
	myConfig.cFont = CD_CONFIG_GET_STRING ("Configuration", "font");
	myData.cFontSize = g_strdup_printf ("%s", myConfig.cFont);
	myData.cFontSize = strrchr(myData.cFontSize, ' ');  // On enlève tout ce qui se trouve avant le dernier 'espace'
	myData.cFontSize = ltrim( myData.cFontSize, " " );  // On supprime l'espace -> Il ne nous reste QUE la taille ;-)	
	cd_debug ("RSSreader-debug : myConfig.cFont = %s",myConfig.cFont);	
	if (g_strstr_len (myConfig.cFont, -1, "Bold") != NULL)
		myData.bFontIsBold = TRUE;
    else
		myData.bFontIsBold = FALSE;
    	
    if (g_strstr_len (myConfig.cFont, -1, "Italic") != NULL)
		myData.bFontIsItalic = TRUE;
    else
		myData.bFontIsItalic = FALSE;		
	// On ne veut garder QUE le nom et pas la taille ou le style dans la variable... Ex: 'Sans' au lieu de 'Sans Bold Italic 12' ;-)
	g_strreverse (myConfig.cFont);
	myConfig.cFont = strrchr(myConfig.cFont, ' ');
	ltrim( myConfig.cFont, " " );
	g_strreverse (myConfig.cFont);
	
	myConfig.iRefreshTime = 60 * CD_CONFIG_GET_INTEGER ("Configuration", "refresh_time");
	myConfig.bInfoBubble = CD_CONFIG_GET_BOOLEAN ("Configuration", "info_bubble");
	myConfig.iDialogsDuration = 1000 * CD_CONFIG_GET_INTEGER ("Configuration", "dialogs_duration");
	myConfig.bLeftClicForDesklet = CD_CONFIG_GET_BOOLEAN ("Configuration", "left_clic_for_desklet");
	
	myConfig.cSpecificWebBrowser = CD_CONFIG_GET_STRING ("Configuration", "specific_web_browser");
	if (myConfig.cSpecificWebBrowser == NULL)
	{
		cd_debug ("RSSreader-debug : No browser in config  -> use the default one");
		myConfig.cSpecificWebBrowser = g_strdup_printf ("xdg-open");
	}
	
	myConfig.cMessageNoTitle = g_strdup_printf (D_("No Title"));
	myConfig.cMessageNoUrl = g_strdup_printf (D_("No RSS feed URL defined..."));
	myConfig.cMessageFailedToConnect = g_strdup_printf (D_("Failed to connect..."));
	
	// Section Appearance
	myConfig.bDisplayLogo = CD_CONFIG_GET_BOOLEAN ("Appearance", "display_logo");
	myConfig.fLogoSize = CD_CONFIG_GET_DOUBLE ("Appearance", "logo_size");
	myConfig.bDisplayBackground = CD_CONFIG_GET_BOOLEAN ("Appearance", "display_background");
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Appearance", "background_color1", myConfig.fBackgroundColor1, couleur);
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Appearance", "background_color2", myConfig.fBackgroundColor2, couleur);
	myConfig.iBackgroundRadius = CD_CONFIG_GET_INTEGER ("Appearance", "background_radius");
	myConfig.iBorderThickness = CD_CONFIG_GET_INTEGER ("Appearance", "border_thickness");
	CD_CONFIG_GET_COLOR_WITH_DEFAULT ("Appearance", "border_color", myConfig.fBorderColor, couleur);
	myConfig.iTitlePositionX = CD_CONFIG_GET_INTEGER ("Appearance", "title_position_x");
	myConfig.iTitlePositionY = CD_CONFIG_GET_INTEGER ("Appearance", "title_position_y");	
	myConfig.iTextPositionX = CD_CONFIG_GET_INTEGER ("Appearance", "text_position_x");
	myConfig.iTextPositionY = CD_CONFIG_GET_INTEGER ("Appearance", "text_position_y");	
	myConfig.iSpaceBetweenLines = CD_CONFIG_GET_INTEGER ("Appearance", "space_between_lines");

	// Other
	myConfig.cLogoPath = CD_CONFIG_GET_FILE_PATH ("Icon", "icon", "icon.svg");	
			
	cd_debug ("RSSreader-debug : myConfig.cUrl = %s",myConfig.cUrl);
	cd_debug ("RSSreader-debug : myConfig.cName = %s",myConfig.cName);
	cd_debug ("RSSreader-debug : myConfig.iLines = %i",myConfig.iLines);
	cd_debug ("RSSreader-debug : myConfig.iTitleNum = %i",myConfig.iTitleNum);
	cd_debug ("RSSreader-debug : myConfig.iDialogsDuration = %i",myConfig.iDialogsDuration);
	
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cSpecificWebBrowser);
	g_free (myConfig.cUrl);
	g_free (myConfig.cName);
	g_free (myConfig.cLogoPath);
	g_free (myConfig.cMessageNoTitle);
	g_free (myConfig.cMessageNoUrl);
	g_free (myConfig.cMessageFailedToConnect);
	// g_free (myConfig.cFont);  // Pourquoi çà plante ? :/
	// g_free (myConfig.cTitleFont);  // Pourquoi çà plante ? :/
		
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pTask);
	cairo_dock_free_task (myData.pTitleTask);
	cairo_dock_free_task (myData.pAutomaticRefreshTask);
	g_free (myData.cLastFirstFeedLine);
	g_free (myData.cLastSecondFeedLine);
	g_free (myData.cTaskBridge);
	g_free (myData.cDialogMessage);	
	//g_free (myData.cTempText);  // Pourquoi çà plante ? :/
	//g_free (myData.cFontSize);  // Pourquoi çà plante ? :/
	//g_free (myData.cTitleFontSize);  // Pourquoi çà plante ? :/
	//g_free (myData.cFeedLine[31]);  // A voir comment faire
	
CD_APPLET_RESET_DATA_END
