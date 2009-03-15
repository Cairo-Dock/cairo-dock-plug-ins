/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey <fabounet@users.berlios.de>

******************************************************************************/
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


static char  *s_cTmpFile = NULL;

void cd_compiz_start_system_wm (void) {
	const gchar * cCommand = NULL;
	if (myConfig.cUserWMCommand != NULL) {
		cCommand = g_strdup_printf("%s &",myConfig.cUserWMCommand);
	}
	else {
		switch (g_iDesktopEnv) {
			case CAIRO_DOCK_GNOME :
			case CAIRO_DOCK_XFCE :
				cCommand = "metacity --replace &";
			break;
			case CAIRO_DOCK_KDE :
				cCommand = "kwin --replace &";
			break ;
			default :
				cd_warning ("couldn't guess system WM");
			return ;
		}
	}
	myData.bCompizRestarted = TRUE;
	cd_compiz_kill_compmgr(); //On tue tout les compositing managers
	
	if (myConfig.cUserWMCommand != NULL)
		system (cCommand); //Obligatoir pour metacity --replace && xcompmngr -f
	else
		cairo_dock_launch_command (cCommand); //Ne prend pas les commandes avec des &
		
	cd_message ("Compiz - Run: %s ", cCommand);
}

void cd_compiz_start_compiz (void) {
	GString *sCommand = g_string_new ("");
	
	gchar *cCompizBin = NULL; //On cherche
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
	cd_compiz_acquisition ();
	
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
	g_return_if_fail (iDecorator >= 0 && iDecorator < COMPIZ_NB_DECORATORS && myConfig.cDecorators[iDecorator] != NULL);
	gchar *cCommand = g_strdup_printf ("%s --replace", myConfig.cDecorators[iDecorator]);
	myData.bDecoratorRestarted = TRUE;
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}

void cd_compiz_kill_compmgr(void) {
	gchar *cCommand = g_strdup_printf("bash %s/compiz-kill", MY_APPLET_SHARE_DATA_DIR);
	system (cCommand);
	g_free (cCommand);
}


void cd_compiz_acquisition (void) {
	s_cTmpFile = g_strdup ("/tmp/compiz.XXXXXX");
	int fds =mkstemp (s_cTmpFile);
	if (fds == -1) {
		g_free (s_cTmpFile);
		s_cTmpFile = NULL;
		return;
	}
	gchar *cCommand = g_strdup_printf("bash %s/compiz %s %s", MY_APPLET_SHARE_DATA_DIR, myConfig.cWindowDecorator, s_cTmpFile);
	system (cCommand);
	g_free (cCommand);
	close(fds);
}

void cd_compiz_read_data(void) {
	if (s_cTmpFile == NULL)
		return ;
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(s_cTmpFile, &cContent, &length, &erreur);
	if (erreur != NULL) {
		cd_warning ("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		myData.bAcquisitionOK = FALSE;
	}
	else {
		myData.bCompizIsRunning = (cContent[0] == '1');
		myData.bDecoratorIsRunning = (cContent[0] != '\0' && cContent[1] == '1');
		g_free (cContent);
		myData.bAcquisitionOK = TRUE;
	}
	g_remove (s_cTmpFile);
	g_free (s_cTmpFile);
	s_cTmpFile = NULL;
}

gboolean cd_compiz_update_from_data (void) {
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
	return myData.bAcquisitionOK;
}
