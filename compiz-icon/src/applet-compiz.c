/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey <fabounet@users.berlios.de>

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icon.h"
#include "applet-compiz.h"

CD_APPLET_INCLUDE_MY_VARS

#define CD_COMPIZ_TMP_FILE "/tmp/compiz"
#define CD_COMPIZ_CHECK_TIME 5000

static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;
static GStaticMutex mutexData = G_STATIC_MUTEX_INIT;


void cd_compiz_start_system_wm (void) {
	const gchar * cCommand = NULL;
	if (myConfig.cUserWMCommand != NULL) {
		cCommand = g_strdup_printf ("%s &", myConfig.cUserWMCommand);
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
	if (myConfig.cUserWMCommand != NULL) {
	  system(cCommand);
	}
	else {
	  cairo_dock_launch_command (cCommand);
	}
	cd_message ("Compiz - Run: %s ", cCommand);
	g_free(cCommand);
}

void cd_compiz_start_compiz (void) {
	GString *sCommand = g_string_new ("");
	g_string_assign (sCommand, "compiz.real --replace --ignore-desktop-hints ccp");
	if (myConfig.lBinding) {
		g_string_append (sCommand, " --loose-binding");
	}
	if (myConfig.iRendering) {
		g_string_append (sCommand, " --indirect-rendering");
	}
	
	if (strcmp (myConfig.cWindowDecorator, "emerald") != 0)
		g_string_append (sCommand, " --sm-disable");  // pas de '&' a la fin.
	cd_debug ("%s (%s)", __func__, sCommand->str);
	
	myData.bCompizRestarted = TRUE;
	cd_compiz_kill_compmgr(); //On tue tout les compositing managers
	cairo_dock_launch_command (sCommand->str);
	
	g_string_free (sCommand, TRUE);
	
	cd_compiz_start_favorite_decorator ();  // ca ne marche pas si on ecrit quelque chose du genre "compiz && emerald".
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
	gchar *cCommand = g_strdup_printf("bash %s/compiz %s", MY_APPLET_SHARE_DATA_DIR, myConfig.cWindowDecorator);
	system (cCommand);
	g_free (cCommand);
}

void cd_compiz_read_data(void) {
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(CD_COMPIZ_TMP_FILE, &cContent, &length, &erreur);
	if (erreur != NULL)	{
		cd_warning("Attention : %s", erreur->message);
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
}

void cd_compiz_update_from_data (void) {
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
}
