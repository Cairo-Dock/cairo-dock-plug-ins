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

#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-composite-manager.h"
#include "applet-config.h"


static void _cd_on_keybinding_pull (const char *keystring, gpointer user_data)
{
	cd_toggle_composite ();
}

CD_APPLET_GET_CONFIG_BEGIN
	myConfig.cWmCompositor = CD_CONFIG_GET_STRING ("Configuration", "compositor");
	myConfig.cWmFallback = CD_CONFIG_GET_STRING ("Configuration", "fallback");
	
	myConfig.cIconCompositeON = CD_CONFIG_GET_STRING ("Configuration", "icon on");
	myConfig.cIconCompositeOFF = CD_CONFIG_GET_STRING ("Configuration", "icon off");
	
	myConfig.bAskBeforeSwitching = CD_CONFIG_GET_BOOLEAN ("Configuration", "ask");
	
	myConfig.iActionOnMiddleClick = CD_CONFIG_GET_INTEGER ("Configuration", "action on click");
	
	myConfig.cShortCut = CD_CONFIG_GET_STRING ("Configuration", "shortkey");
	cd_keybinder_bind (myConfig.cShortCut, (CDBindkeyHandler)_cd_on_keybinding_pull, (gpointer)NULL);
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	g_free (myConfig.cWmCompositor);
	g_free (myConfig.cWmFallback);
	g_free (myConfig.cIconCompositeON);
	g_free (myConfig.cIconCompositeOFF);
	if (myConfig.cShortCut)
	{
		cd_keybinder_unbind(myConfig.cShortCut, (CDBindkeyHandler)_cd_on_keybinding_pull);
		g_free (myConfig.cShortCut);
	}
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	
CD_APPLET_RESET_DATA_END
