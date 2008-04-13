/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@hollowproject.org)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-load-icon.h"
#include "applet-compiz.h"

CD_APPLET_INCLUDE_MY_VARS

#define CD_COMPIZ_TMP_FILE "/tmp/compiz"
#define CD_COMPIZ_CHECK_TIME 2000

static int s_iThreadIsRunning = 0;
static int s_iSidTimerRedraw = 0;
static GStaticMutex mutexData = G_STATIC_MUTEX_INIT;


void cd_compiz_start_system_wm (void)
{
	const gchar * cCommand = NULL;
	if (myConfig.cUserWMCommand != NULL)
	{
		cCommand = myConfig.cUserWMCommand;
	}
	else {
		switch (g_iDesktopEnv)
		{
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
	cairo_dock_launch_command (cCommand);
}

void cd_compiz_start_compiz (void)
{
	GString *sCommand = g_string_new ("");
	g_string_assign (sCommand, "compiz.real --replace --ignore-desktop-hints ccp");
	//cmd = "compiz.real --replace --ignore-desktop-hints ccp";
	if (myConfig.lBinding) {
		//cmd = g_strdup_printf("%s --loose-binding", cmd);
		g_string_append (sCommand, " --loose-binding");
	}
	if (myConfig.iRendering) {
		//cmd = g_strdup_printf("%s --indirect-rendering", cmd);
		g_string_append (sCommand, " --indirect-rendering");
	}
	
	if (strcmp (myConfig.cWindowDecorator, "emerald") != 0)
		g_string_append (sCommand, " --sm-disable");  // pas de '&' a la fin.
	//cmd = g_strdup_printf("%s &", cmd);
	cd_debug ("%s (%s)", __func__, sCommand->str);
	
	myData.bCompizRestarted = TRUE;
	cd_compiz_kill_compmgr(); //On tue tout les compositing managers
	cairo_dock_launch_command (sCommand->str);
	
	g_string_free (sCommand, TRUE);
	
	cd_compiz_start_favorite_decorator ();  // ca ne marche pas si on ecrit quelque chose du genre "compiz && emerald".
}

void cd_compiz_switch_manager(void)
{
	cd_compiz_get_data ();
	
	gboolean bAcquisitionOK = cd_compiz_read_data ();
	if (bAcquisitionOK)
	{
		if (myData.bCompizIsRunning)
			cd_compiz_start_system_wm ();
		else
			cd_compiz_start_compiz ();
	}
}


void cd_compiz_start_favorite_decorator (void)
{
	g_print ("%s (%s)\n", __func__, myConfig.cWindowDecorator);
	gchar *cCommand = g_strdup_printf ("%s --replace", myConfig.cWindowDecorator);
	myData.bDecoratorRestarted = TRUE;
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}

void cd_compiz_start_decorator (compizDecorator iDecorator)
{
	g_print ("%s (%d)\n", __func__, iDecorator);
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




static gboolean _cd_compiz_check_for_redraw (gpointer data)
{
	int iThreadIsRunning = g_atomic_int_get (&s_iThreadIsRunning);
	cd_message ("%s (%d)", __func__, iThreadIsRunning);
	if (! iThreadIsRunning)
	{
		s_iSidTimerRedraw = 0;
		if (myIcon == NULL)
		{
			cd_warning ("annulation du chargement de Compiz-Icon");
			return FALSE;
		}
		
		g_static_mutex_lock (&mutexData);
		
		cd_compiz_update_main_icon ();
		
		//g_print (" etat : %d - %d / action : %d - %d\n", myData.bCompizIsRunning, myData.bDecoratorIsRunning, myData.bCompizRestarted, myData.bDecoratorRestarted);
		
		if (! myData.bCompizIsRunning && myConfig.bAutoReloadCompiz && ! myData.bCompizRestarted)
		{
			myData.bCompizRestarted = TRUE;  // c'est nous qui l'avons change.
			cd_compiz_start_compiz ();  // relance aussi le decorateur.
		}
		else if (! myData.bDecoratorIsRunning && myConfig.bAutoReloadDecorator && ! myData.bDecoratorRestarted)
		{
			myData.bDecoratorRestarted = TRUE;  // c'est nous qui l'avons change.
			cd_compiz_start_favorite_decorator ();
		}
		
		if (myData.bCompizIsRunning)
			myData.bCompizRestarted = FALSE;  // compiz tourne, on le relancera s'il plante.
		if (myData.bDecoratorIsRunning)
			myData.bDecoratorRestarted = FALSE;  // le decorateur tourne, on le relancera s'il plante.
		
		g_static_mutex_unlock (&mutexData);
		
		//\_______________________ On lance le timer si necessaire.
		if (myData.iSidTimer == 0)
			myData.iSidTimer = g_timeout_add (CD_COMPIZ_CHECK_TIME, (GSourceFunc) cd_compiz_timer, NULL);
		return FALSE;
	}
	return TRUE;
}


gboolean cd_compiz_timer (gpointer data) {
	cd_compiz_launch_measure();
	return TRUE;
}

void cd_compiz_launch_measure(void) {
	cd_message ("");
	if (g_atomic_int_compare_and_exchange (&s_iThreadIsRunning, 0, 1)) {  //il etait egal a 0, on lui met 1 et on lance le thread.
		cd_message (" ==> lancement du thread de calcul");
		
		if (s_iSidTimerRedraw == 0) {
			s_iSidTimerRedraw = g_timeout_add (333, (GSourceFunc) _cd_compiz_check_for_redraw, (gpointer) NULL);
		}
		
		GError *erreur = NULL;
		GThread* pThread = g_thread_create ((GThreadFunc) cd_compiz_threaded_calculation, NULL, FALSE, &erreur);
		if (erreur != NULL) {
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
		}
	}
}

gpointer cd_compiz_threaded_calculation (gpointer data) {
	cd_compiz_get_data();
	
	g_static_mutex_lock (&mutexData);
	myData.bAcquisitionOK = cd_compiz_read_data();
	g_static_mutex_unlock (&mutexData);
	
	g_atomic_int_set (&s_iThreadIsRunning, 0);
	cd_message ("*** fin du thread compiz");
	return NULL;
}

void cd_compiz_get_data(void) {
	gchar *cCommand = g_strdup_printf("bash %s/compiz %s", MY_APPLET_SHARE_DATA_DIR, myConfig.cWindowDecorator);
	system (cCommand);
	g_free (cCommand);
}

static void _compiz_get_values_from_file (gchar *cContent) {
	myData.bCompizIsRunning = (cContent[0] == '1');
	myData.bDecoratorIsRunning = (cContent[0] != '\0' && cContent[1] == '1');
}

gboolean cd_compiz_read_data(void) {
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents(CD_COMPIZ_TMP_FILE, &cContent, &length, &erreur);
	if (erreur != NULL)	{
		cd_warning("Attention : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
		return FALSE;
	}
	else {
		_compiz_get_values_from_file (cContent);
		g_free (cContent);
		return TRUE;
	}
}
