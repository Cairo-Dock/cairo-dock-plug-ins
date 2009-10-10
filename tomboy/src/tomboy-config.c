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

#include "tomboy-struct.h"
#include "tomboy-dbus.h"
#include "tomboy-config.h"


CD_APPLET_GET_CONFIG_BEGIN
	myConfig.defaultTitle		= CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cIconDefault 		= CD_CONFIG_GET_STRING ("Icon", "default icon");
	myConfig.cIconClose		= CD_CONFIG_GET_STRING ("Icon", "close icon");
	myConfig.cIconBroken 		= CD_CONFIG_GET_STRING ("Icon", "broken icon");
	myConfig.cIconEmpty 		= CD_CONFIG_GET_STRING ("Icon", "empty icon");
	myConfig.bNoDeletedSignal 	= CD_CONFIG_GET_BOOLEAN ("Configuration", "no deleted signal"); // Ce problème n'étant vu que sur Gutsy, on pourrait le supprimer
	myConfig.iAppControlled		= CD_CONFIG_GET_INTEGER ("Configuration", "app controlled");
	myConfig.cRenderer 		= CD_CONFIG_GET_STRING ("Configuration", "renderer");
	myConfig.bDrawContent 		= CD_CONFIG_GET_BOOLEAN ("Configuration", "draw content");
	myConfig.bPopupContent 		= CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "popup content", TRUE);
	myConfig.cDateFormat 		= CD_CONFIG_GET_STRING ("Configuration", "date format");
	myConfig.iDialogDuration 	= 1000 * CD_CONFIG_GET_INTEGER_WITH_DEFAULT ("Configuration", "time_dialog", 3);
	if (myConfig.cDateFormat == NULL)
		myConfig.cDateFormat = g_strdup ("%d/%m/%y");
	myConfig.bAutoNaming = CD_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Configuration", "auto-naming", TRUE);
	myConfig.bAskBeforeDelete = CD_CONFIG_GET_BOOLEAN ("Configuration", "ask delete");
	double couleur[3] = {1., 0., 0.};
	CD_CONFIG_GET_COLOR_RVB_WITH_DEFAULT ("Configuration", "text color", myConfig.fTextColor, couleur);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.defaultTitle);
	
	g_free (myConfig.cIconDefault);
	g_free (myConfig.cIconClose);
	g_free (myConfig.cIconBroken);
	g_free (myConfig.cIconEmpty);
	g_free (myConfig.cRenderer);
	g_free (myConfig.cDateFormat);
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	cairo_dock_free_task (myData.pTask);
	cairo_surface_destroy (myData.pSurfaceDefault);
	cairo_surface_destroy (myData.pSurfaceNote);
	
	free_all_notes ();  // detruit aussi la liste des icones.
	g_hash_table_destroy (myData.hNoteTable);
	
CD_APPLET_RESET_DATA_END
