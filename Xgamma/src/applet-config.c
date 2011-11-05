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


CD_APPLET_GET_CONFIG_BEGIN
	//\_________________ On recupere toutes les valeurs de notre fichier de conf.
	myConfig.iScrollVariation = CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "scroll_variation", 5);
	myConfig.fInitialGamma = CD_CONFIG_GET_DOUBLE ("Configuration", "initial gamma");
	myConfig.cDefaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cShortkey = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	myConfig.cShortkey2 = CD_CONFIG_GET_STRING ("Configuration", "shortkey2");
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN	
	g_free (myConfig.cDefaultTitle);
	g_free (myConfig.cShortkey);
	g_free (myConfig.cShortkey2);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (myData.pDialog)
	{
		cairo_dock_dialog_unreference (myData.pDialog);  // detruit aussi le widget interactif.
	}
	else
	{
		gtk_widget_destroy (myData.pWidget);
	}
CD_APPLET_RESET_DATA_END
