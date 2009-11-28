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
#include "applet-config.h"


//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.bShowKbdIndicator = CD_CONFIG_GET_BOOLEAN ("Configuration", "show indic");
	myConfig.iTransitionDuration = CD_CONFIG_GET_INTEGER ("Configuration", "transition");
	myConfig.fTextRatio = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "text ratio", 1.);
	CD_CONFIG_GET_COLOR_RVB("Configuration", "text color", myConfig.textDescription.fColorStart);
	CD_CONFIG_GET_COLOR_RVB("Configuration", "text color", myConfig.textDescription.fColorStop);
	
	gchar *cFontDescription = CD_CONFIG_GET_STRING ("Configuration", "font");
	if (cFontDescription == NULL)
	{
		cFontDescription = g_strdup ("Sans");  // sinon fd est NULL.
	}
	PangoFontDescription *fd = pango_font_description_from_string (cFontDescription);
	myConfig.textDescription.cFont = g_strdup (pango_font_description_get_family (fd));
	myConfig.textDescription.iWeight = pango_font_description_get_weight (fd);
	myConfig.textDescription.iStyle = pango_font_description_get_style (fd);
	if (pango_font_description_get_size (fd) == 0)  // anciens parametres de font.
	{
		int iWeight = g_key_file_get_integer (pKeyFile, "Configuration", "text weight", NULL);
		myConfig.textDescription.iWeight = cairo_dock_get_pango_weight_from_1_9 (iWeight);
		myConfig.textDescription.iStyle = PANGO_STYLE_NORMAL;
		
		pango_font_description_set_size (fd, 12 * PANGO_SCALE);
		pango_font_description_set_weight (fd, myConfig.textDescription.iWeight);
		pango_font_description_set_style (fd, myConfig.textDescription.iStyle);
		g_free (cFontDescription);
		cFontDescription = pango_font_description_to_string (fd);
		g_key_file_set_string (pKeyFile, "Configuration", "font", cFontDescription);
	}
	pango_font_description_free (fd);
	g_free (cFontDescription);
	
	myConfig.textDescription.bOutlined = CD_CONFIG_GET_BOOLEAN ("Configuration", "outlined");
	
	myConfig.cBackgroundImage = CD_CONFIG_GET_STRING ("Configuration", "bg image");
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cBackgroundImage);
	g_free (myConfig.textDescription.cFont);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	if (myData.pBackgroundSurface != NULL)
		cairo_surface_destroy (myData.pBackgroundSurface);
	if (myData.iBackgroundTexture != 0)
		_cairo_dock_delete_texture (myData.iBackgroundTexture);
	if (myData.pOldSurface != NULL)
		cairo_surface_destroy (myData.pOldSurface);
	if (myData.iOldTexture != 0)
		_cairo_dock_delete_texture (myData.iOldTexture);
	if (myData.pCurrentSurface != NULL)
		cairo_surface_destroy (myData.pCurrentSurface);
	if (myData.iCurrentTexture != 0)
		_cairo_dock_delete_texture (myData.iCurrentTexture);
CD_APPLET_RESET_DATA_END
