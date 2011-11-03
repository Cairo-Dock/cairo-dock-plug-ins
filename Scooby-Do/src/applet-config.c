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
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-session.h"
#include "applet-appli-finder.h"
#include "applet-listing.h"
#include "applet-search.h"
#include "applet-config.h"

static const gchar *s_DefaultApplis[26+1] = {
	"abiword",
	"gnome-background-properties",
	"gnome-calculator",
	"gnome-dictionary",
	"evince",
	"firefox",
	"gjiten",
	"",
	"inkscape",
	"",
	"kate",
	"",
	"",
	"nautilus --browser",
	"ooffice",
	"pan",
	"",
	"rhythmbox",
	"synaptic",
	"gnome-terminal",
	"",
	"vlc",
	"",
	"",
	"yast",
	"",
	NULL};

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cShortkeySearch = CD_CONFIG_GET_STRING ("Configuration", "shortkey search");
	myConfig.iAppearanceDuration = CD_CONFIG_GET_INTEGER ("Configuration", "appear duration");
	myConfig.iCloseDuration = CD_CONFIG_GET_INTEGER ("Configuration", "stop duration");
	myConfig.cIconAnimation = CD_CONFIG_GET_STRING ("Configuration", "animation");
	CD_CONFIG_GET_COLOR ("Configuration", "frame color", myConfig.pFrameColor);
	
	myConfig.fFontSizeRatio = CD_CONFIG_GET_DOUBLE ("Configuration", "font size");
	myConfig.bTextOnTop = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "text on top", TRUE);
	
	gchar *cFontDescription = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", "font", "Monospace 16");
	if (cFontDescription == NULL)
		cFontDescription = g_strdup ("Monospace 16");  // sinon fd est NULL.
	
	PangoFontDescription *fd = pango_font_description_from_string (cFontDescription);
	myConfig.labelDescription.cFont = g_strdup (pango_font_description_get_family (fd));
	myConfig.labelDescription.iWeight = pango_font_description_get_weight (fd);
	myConfig.labelDescription.iStyle = pango_font_description_get_style (fd);
	pango_font_description_free (fd);
	g_free (cFontDescription);
	
	myConfig.labelDescription.bOutlined = CD_CONFIG_GET_BOOLEAN ("Configuration", "text outlined");
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "text color", myConfig.labelDescription.fColorStart);
	CD_CONFIG_GET_COLOR_RVB ("Configuration", "text color", myConfig.labelDescription.fColorStop);
	myConfig.labelDescription.iMargin = 2;
	CD_CONFIG_GET_COLOR ("Configuration", "bg color", myConfig.labelDescription.fBackgroundColor);
	
	myConfig.iNbResultMax = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb results", 50);
	myConfig.iNbLinesInListing = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nb lines", 10);
	
	myConfig.infoDescription.cFont = g_strdup ("Sans");
	myConfig.infoDescription.iSize = 14;
	myConfig.infoDescription.iWeight = cairo_dock_get_pango_weight_from_1_9 (5);
	myConfig.infoDescription.bOutlined = FALSE;
	myConfig.infoDescription.fColorStart[0] = 1;
	myConfig.infoDescription.fColorStart[1] = 0;
	myConfig.infoDescription.fColorStart[2] = 0;
	myConfig.infoDescription.fColorStop[0] = 1;
	myConfig.infoDescription.fColorStop[1] = 0;
	myConfig.infoDescription.fColorStop[2] = 0;
	myConfig.infoDescription.iStyle = PANGO_STYLE_NORMAL;
	myConfig.infoDescription.fBackgroundColor[3] = 1;
	myConfig.infoDescription.fBackgroundColor[3] = 0;
	myConfig.infoDescription.fBackgroundColor[3] = 0;
	myConfig.infoDescription.fBackgroundColor[3] = 0.33;
	myConfig.infoDescription.iMargin = 8;
	
	myConfig.cPreferredApplis = g_new0 (gchar *, 26+1);  // NULL-terminated
	gchar key[2];
	int i;
	for (i = 0; i < 26; i ++)
	{
		sprintf (key, "%c", 'a'+i);
		myConfig.cPreferredApplis[i] = CD_CONFIG_GET_STRING_WITH_DEFAULT ("Configuration", key, s_DefaultApplis[i]);
	}
	
	myConfig.bUseFiles = CD_CONFIG_GET_BOOLEAN ("Search Engines", "files");
	myConfig.bUseFirefox = CD_CONFIG_GET_BOOLEAN ("Search Engines", "firefox");
	myConfig.bUseRecent = CD_CONFIG_GET_BOOLEAN ("Search Engines", "recent");
	myConfig.bUseWeb = CD_CONFIG_GET_BOOLEAN ("Search Engines", "web");
	myConfig.bUseCommand = CD_CONFIG_GET_BOOLEAN ("Search Engines", "cmd");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cShortkeySearch);
	g_free (myConfig.cIconAnimation);
	g_free (myConfig.labelDescription.cFont);
	g_strfreev (myConfig.cPreferredApplis);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cd_do_close_session ();
	cd_do_exit_session ();
	
	cd_do_free_all_backends ();
	
	cd_do_reset_applications_list ();
	
	cd_do_destroy_listing (myData.pListing);
	
	if (myData.pPromptSurface != NULL)
		cairo_surface_destroy (myData.pPromptSurface);
	if (myData.iPromptTexture != 0)
		_cairo_dock_delete_texture (myData.iPromptTexture);
	if (myData.pScoobySurface != NULL)
		cairo_surface_destroy (myData.pScoobySurface);
	if (myData.pActiveButtonSurface != NULL)
		cairo_surface_destroy (myData.pActiveButtonSurface);
	if (myData.pInactiveButtonSurface != NULL)
		cairo_surface_destroy (myData.pInactiveButtonSurface);
CD_APPLET_RESET_DATA_END
