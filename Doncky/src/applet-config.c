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
#include "applet-xml.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	double couleur[4] = {0., 0., 0.5, 1.};
	
	//\___________________ Section Configuration
	myConfig.cXmlFilePath = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "xml_filepath", "");
	if (myConfig.cXmlFilePath == NULL)
		myConfig.cXmlFilePath = g_strdup_printf ("%s/default.xml", MY_APPLET_SHARE_DATA_DIR);
	
	//\___________________ Section Appearance	
	// decoration
	myConfig.bDisplayBackground 	= CD_CONFIG_GET_BOOLEAN ("Appearance", "display_background");
	CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Appearance", "background_color1", myConfig.fBackgroundColor1, couleur);
	CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Appearance", "background_color2", myConfig.fBackgroundColor2, couleur);
	myConfig.iBackgroundRadius 	= CD_CONFIG_GET_INTEGER ("Appearance", "background_radius");
	myConfig.iBorderThickness 	= CD_CONFIG_GET_INTEGER ("Appearance", "border_thickness");
	CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Appearance", "border_color", myConfig.fBorderColor, couleur);
	
	// texte
	CD_CONFIG_GET_COLOR_RGBA_WITH_DEFAULT ("Appearance", "default_text_color", myConfig.fDefaultTextColor, couleur);
	myConfig.cDefaultFont = CD_CONFIG_GET_STRING ("Appearance", "default_font");  // avec Pango il suffit de lui passer la chaine issue du GtkFontWidget.
	myConfig.iTextMargin = CD_CONFIG_GET_INTEGER ("Appearance", "text_margin");  // marge du texte en haut et à gauche/droite suivant l'alignement.
	myConfig.iSpaceBetweenLines = CD_CONFIG_GET_INTEGER ("Appearance", "space_between_lines");
		
	myConfig.iCheckInterval = 1; // Delay en en seconde
	
	
	// REPRIS DE SYSTEM-MONITOR: (on impose les valeurs)	
	myConfig.fUserHZ = 100;	
	myConfig.bShowCpu = TRUE;
	myConfig.bShowRam = TRUE;
	myConfig.bShowSwap = TRUE;
	myConfig.bShowNvidia = FALSE;
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cDefaultFont);
	g_free (myConfig.cDeskletSize);
	g_free (myConfig.cXmlFilePath);
	g_free (myConfig.cSystemMonitorClass);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cd_doncky_free_item_list (myApplet);

	cairo_dock_stop_task (myData.pPeriodicRefreshTask);
	cairo_dock_free_task (myData.pPeriodicRefreshTask);
	
	// REPRIS DE SYSTEM-MONITOR:
	g_timer_stop (myData.pClock);
	g_timer_destroy (myData.pClock);
	CD_APPLET_REMOVE_MY_DATA_RENDERER;
	
	// On libère les gchar* de myData:
	g_free (myData.cThemeFolder);
	g_free (myData.cXmlFileName);
	g_free (myData.cPrevAlignWidth);
	g_free (myData.cPrevAlignHeight);
	g_free (myData.cPrevFont);
	g_free (myData.cCurrentText);	
	g_free (myData.cLastAlignHeight);
	g_free (myData.cModelName);
	g_free (myData.cGPUName);
	g_free (myData.cDriverVersion);

CD_APPLET_RESET_DATA_END
