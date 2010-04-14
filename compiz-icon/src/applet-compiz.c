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

#define _BSD_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icon.h"
#include "applet-compiz.h"


//static char  *s_cTmpFile = NULL;

void cd_compiz_start_system_wm (void) {
	const gchar * cCommand = NULL;
	if (myConfig.cUserWMCommand != NULL) {
		cCommand = myConfig.cUserWMCommand;
	}
	else {
		switch (g_iDesktopEnv) {
			case CAIRO_DOCK_GNOME :
			case CAIRO_DOCK_XFCE :
				cCommand = "metacity --replace";
			break;
			case CAIRO_DOCK_KDE :
				cCommand = "kwin --replace";
			break ;
			default :
				cd_warning ("couldn't guess system WM");
			return ;
		}
	}
	myData.bCompizRestarted = TRUE;
	cd_compiz_kill_compmgr(); //On tue tout les compositing managers
	
	cd_message ("Compiz - Run: %s ", cCommand);
	cairo_dock_launch_command (cCommand);
}

void cd_compiz_start_compiz (void) {
	GString *sCommand = g_string_new ("");
	
	const gchar *cCompizBin = NULL; //On cherche
	if (g_file_test ("/usr/bin/compiz.real", G_FILE_TEST_EXISTS))
		cCompizBin = "compiz.real";
	else
		cCompizBin = "compiz";
		
	g_string_printf (sCommand, "%s --replace --ignore-desktop-hints ccp", cCompizBin);
	if (myConfig.lBinding) {
		g_string_append (sCommand, " --loose-binding");
	}
	if (myConfig.iRendering) {
		g_string_append (sCommand, " --indirect-rendering");
	}
	if (myConfig.uLocalScreen) {
	 g_string_append (sCommand, " --only-current-screen");
	}
	
	if (strcmp (myConfig.cWindowDecorator, "emerald") != 0)
		g_string_append (sCommand, " --sm-disable");
	
	cd_debug ("%s (%s)", __func__, sCommand->str);
	
	myData.bCompizRestarted = TRUE;
	cd_compiz_kill_compmgr(); //On tue tout les compositing managers
	cairo_dock_launch_command (sCommand->str);
	
	g_string_free (sCommand, TRUE);
	
	cd_compiz_start_favorite_decorator ();
}

void cd_compiz_switch_manager(void) {
	cd_compiz_read_data ();
	if (myData.bAcquisitionOK) {
		if (myData.bCompizIsRunning)
			cd_compiz_start_system_wm ();
		else
			cd_compiz_start_compiz ();
	}
}


void cd_compiz_start_favorite_decorator (void) {
	cd_debug ("%s (%s)", __func__, myConfig.cWindowDecorator);
	gchar *cCommand = g_strdup_printf ("%s --replace", myConfig.cWindowDecorator);
	myData.bDecoratorRestarted = TRUE;
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}

void cd_compiz_start_decorator (compizDecorator iDecorator) {
	cd_debug ("%s (%d)", __func__, iDecorator);
	g_return_if_fail (iDecorator < COMPIZ_NB_DECORATORS && myConfig.cDecorators[iDecorator] != NULL);
	gchar *cCommand = g_strdup_printf ("%s --replace", myConfig.cDecorators[iDecorator]);
	myData.bDecoratorRestarted = TRUE;
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}

void cd_compiz_kill_compmgr(void) {
	gchar *cCommand = g_strdup_printf("bash %s/compiz-kill", MY_APPLET_SHARE_DATA_DIR);
	int r = system (cCommand);
	g_free (cCommand);
}


void cd_compiz_read_data(void) {
	gchar *cCommand = g_strdup_printf("sh %s/compiz %s", MY_APPLET_SHARE_DATA_DIR, myConfig.cWindowDecorator);  // truc de ouf : sans le 'sh' devant ca renvoit toujours '10' !
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult == NULL)
	{
		myData.bAcquisitionOK = FALSE;
		return ;
	}
	
	myData.bCompizIsRunning = (cResult[0] == '1');
	myData.bDecoratorIsRunning = (cResult[0] != '\0' && cResult[1] == '1');
	//g_print ("compiz : '%s' => %d/%d\n", standard_output, myData.bCompizIsRunning, myData.bDecoratorIsRunning);
	myData.bAcquisitionOK = TRUE;
}

gboolean cd_compiz_update_from_data (void) {
	CD_APPLET_ENTER;
	cd_compiz_update_main_icon ();
	cd_debug ("Compiz: %d - Decorator: %d", myData.bCompizIsRunning, myData.bDecoratorIsRunning);
	if (! myData.bCompizIsRunning && myConfig.bAutoReloadCompiz) {
		if (! myData.bCompizRestarted) {
			myData.bCompizRestarted = TRUE;  // c'est nous qui l'avons change.
			cd_compiz_start_compiz ();  // relance compiz.
		}
	}
	if (! myData.bDecoratorIsRunning && myConfig.bAutoReloadDecorator) {
		if (! myData.bDecoratorRestarted) {
			myData.bDecoratorRestarted = TRUE;  // c'est nous qui l'avons change.
			cd_compiz_start_favorite_decorator (); // relance aussi le decorateur.
		}
	}
	
	if (myData.bCompizIsRunning)
		myData.bCompizRestarted = FALSE;  // Compiz tourne, on le relancera s'il plante.
	if (myData.bDecoratorIsRunning)
		myData.bDecoratorRestarted = FALSE;  // le decorateur tourne, on le relancera s'il plante.
	CD_APPLET_LEAVE (myData.bAcquisitionOK);
	//return myData.bAcquisitionOK;
}
