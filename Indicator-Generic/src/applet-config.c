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

#define IND_GEN_EXCEPTIONS_HARD "libapplication.so;libappmenu.so;libdatetime.so;libmessaging.so;libsoundmenu.so"

//\_________________ Here you have to get all your parameters from the conf file. Use the macros CD_CONFIG_GET_BOOLEAN, CD_CONFIG_GET_INTEGER, CD_CONFIG_GET_STRING, etc. myConfig has been reseted to 0 at this point. This function is called at the beginning of init and reload.
CD_APPLET_GET_CONFIG_BEGIN
	myConfig.defaultTitle = CD_CONFIG_GET_STRING ("Icon", "name");
	myConfig.cIndicatorName = CD_CONFIG_GET_STRING ("Configuration", "indicator");

	// if the indicator is not null/empty, we have the launcher
	if (myConfig.cIndicatorName == NULL || *myConfig.cIndicatorName == '\0')
	{
		gchar *cExceptionsHard = CD_CONFIG_GET_STRING ("Configuration", "exceptions");
		if (cExceptionsHard == NULL)
			cExceptionsHard = g_strdup (IND_GEN_EXCEPTIONS_HARD);
		gchar *cExceptionsEditable = CD_CONFIG_GET_STRING ("Configuration", "except-edit");
		// merge
		gchar *cExceptions = g_strdup_printf ("%s;%s", cExceptionsHard, cExceptionsEditable);

		myConfig.cExceptionsList = g_strsplit (cExceptions, ";", -1);
		if (myConfig.cExceptionsList[0] == NULL || *myConfig.cExceptionsList[0] == '\0')
		{
			g_strfreev (myConfig.cExceptionsList);
			myConfig.cExceptionsList = NULL;
		}
		g_free (cExceptions);
		g_free (cExceptionsEditable);
		g_free (cExceptionsHard);
	}
CD_APPLET_GET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myConfig. This one will be reseted to 0 at the end of this function. This function is called right before you get the applet's config, and when your applet is stopped, in the end.
CD_APPLET_RESET_CONFIG_BEGIN
	g_strfreev (myConfig.cExceptionsList);
	g_free (myConfig.defaultTitle);
	g_free (myConfig.cIndicatorName);
CD_APPLET_RESET_CONFIG_END


//\_________________ Here you have to free all ressources allocated for myData. This one will be reseted to 0 at the end of this function. This function is called when your applet is stopped, in the very end.
CD_APPLET_RESET_DATA_BEGIN
	g_list_free (myData.pIndicatorsList); // no need to free the data inside
	
CD_APPLET_RESET_DATA_END
