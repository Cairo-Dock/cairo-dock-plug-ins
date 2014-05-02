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
	myConfig.cShortkey = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	myConfig.bShowKbdIndicator = CD_CONFIG_GET_BOOLEAN ("Configuration", "show indic");
	myConfig.cEmblemNumLock = CD_CONFIG_GET_STRING ("Configuration", "emblem numlock");
	myConfig.cEmblemCapsLock = CD_CONFIG_GET_STRING ("Configuration", "emblem capslock");
	myConfig.iTransitionDuration = CD_CONFIG_GET_INTEGER ("Configuration", "transition");
	myConfig.fTextRatio = CD_CONFIG_GET_DOUBLE_WITH_DEFAULT ("Configuration", "text ratio", 1.);
	
	gboolean bUseDefaultColors = (CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "style", 1) == 0);
	gboolean bCustomFont = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "custom font", FALSE);  // false by default
	if (! bUseDefaultColors && bCustomFont)  // custom font
	{
		gchar *cFont = CD_CONFIG_GET_STRING ("Configuration", "font");
		gldi_text_description_set_font (&myConfig.textDescription, cFont);
	}
	else  // use the default font
	{
		gldi_text_description_copy (&myConfig.textDescription, &myStyleParam.textDescription);
	}
	myConfig.textDescription.bNoDecorations = TRUE;
	
	if (! bUseDefaultColors)  // custom colors
	{
		CD_CONFIG_GET_COLOR ("Configuration", "text color", &myConfig.textDescription.fColorStart);
		myConfig.textDescription.bOutlined = CD_CONFIG_GET_BOOLEAN ("Configuration", "outlined");
		myConfig.textDescription.bUseDefaultColors = FALSE;
	}
	else  // else, no outline and keep default colors
		myConfig.textDescription.bUseDefaultColors = TRUE;
	
	myConfig.cBackgroundImage = CD_CONFIG_GET_STRING ("Configuration", "bg image");
	myConfig.iNLetters = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "nLetters", 3);
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cBackgroundImage);
	g_free (myConfig.cEmblemCapsLock);
	g_free (myConfig.cEmblemNumLock);
	gldi_text_description_reset (&myConfig.textDescription);
	g_free (myConfig.cShortkey);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_unload_image_buffer (&myData.bgImage);
	cairo_dock_free_image_buffer (myData.pOldImage);
	cairo_dock_free_image_buffer (myData.pCurrentImage);
	/**if (myData.pOldSurface != NULL)
		cairo_surface_destroy (myData.pOldSurface);
	if (myData.iOldTexture != 0)
		_cairo_dock_delete_texture (myData.iOldTexture);
	if (myData.pCurrentSurface != NULL)
		cairo_surface_destroy (myData.pCurrentSurface);
	if (myData.iCurrentTexture != 0)
		_cairo_dock_delete_texture (myData.iCurrentTexture);*/
	g_free (myData.cEmblemCapsLock);
	g_free (myData.cEmblemNumLock);
	g_free (myData.cShortGroupName);
	g_free (myData.cGroupName);
CD_APPLET_RESET_DATA_END
