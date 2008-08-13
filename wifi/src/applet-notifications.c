#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-wifi.h"
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the wifi applet\n made by ChAnGFu for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	cairo_dock_remove_dialog_if_any (myIcon);
	cd_wifi_bubble();
CD_APPLET_ON_CLICK_END

static void _wifi_recheck_wireless_extension (GtkMenuItem *menu_item, gpointer *data) {
	cairo_dock_stop_measure_timer (myData.pMeasureTimer);
	cairo_dock_launch_measure (myData.pMeasureTimer);
}

static void eth_config(void) {  /// a mettre dans les plug-ins d'integration.
	if (myConfig.cUserCommand != NULL) { //Commande utilisateur, pour hardy heron ^^
		cairo_dock_launch_command (myConfig.cUserCommand);
		return;
	}

	if (g_file_test ("/opt/wicd/daemon.py", G_FILE_TEST_EXISTS)) { //On détecte wicd
		cairo_dock_launch_command ("/opt/wicd/./gui.py");
		return;
	}
	
	gchar *cCommand = NULL;
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME || g_iDesktopEnv == CAIRO_DOCK_XFCE) {
		int iMajor, iMinor, iMicro;
		cairo_dock_get_gnome_version (&iMajor, &iMinor, &iMicro);
		if (iMajor == 2 && iMinor < 22)
			cCommand = "gksu network-admin";
		else
			cCommand = "network-admin";
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE) { //Ajouter les lignes de KDE
		//cCommand = 
	}
	
	cairo_dock_launch_command (cCommand);
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Wifi", pSubMenu, CD_APPLET_MY_MENU)
		if (! myData.bAcquisitionOK) {
			CD_APPLET_ADD_IN_MENU (D_("Check for Wireless Extension"), _wifi_recheck_wireless_extension, pSubMenu)
		}
		CD_APPLET_ADD_IN_MENU (D_("Network Administration"), eth_config, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

static void toggel_wlan(void) { //Trouver la commande pour activer/désactiver une connection
	GError *erreur = NULL;
	if (myData.bWirelessExt) {
		gchar *cCommand = g_strdup_printf ("gksu ifconfig %s down", myData.cConnName);
		g_spawn_command_line_async (cCommand, &erreur);
		g_free(cCommand);
	}
	else {
		gchar *cCommand = g_strdup_printf ("gksu ifconfig %s up", myData.cConnName);
		g_spawn_command_line_async (cCommand, &erreur);
		g_free(cCommand);
	}
	if (erreur != NULL) {
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	//On ajoutera la désactivation quand elle sera fonctionnelle...
	cairo_dock_launch_measure (myData.pMeasureTimer);
	cairo_dock_remove_dialog_if_any (myIcon);
CD_APPLET_ON_MIDDLE_CLICK_END
