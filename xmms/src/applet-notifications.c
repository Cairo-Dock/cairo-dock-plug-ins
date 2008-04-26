#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-infopipe.h"

CD_APPLET_INCLUDE_MY_VARS


void cd_xmms_prev() {
	GError *erreur = NULL;
	g_free (myData.playingTitle);
	myData.playingTitle = NULL; //Reseting the title to detect it for sure
	switch (myConfig.iPlayer) {
		case MY_XMMS :
			g_spawn_command_line_async ("xmms -r", &erreur);
		break;
		case MY_AUDACIOUS :
			g_spawn_command_line_async ("audacious -r", &erreur);
		break;
		case MY_BANSHEE :
			g_spawn_command_line_async ("banshee --previous", &erreur);
		break;
		case MY_EXAILE :
			g_spawn_command_line_async ("exaile -p", &erreur);
		break;
		default :
		return ;
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'previous on %d' : %s", myConfig.iPlayer, erreur->message);
		g_error_free (erreur);
	}
}

void cd_xmms_pp() {
	GError *erreur = NULL;
	switch (myConfig.iPlayer) {
		case MY_XMMS :
			g_spawn_command_line_async ("xmms -t", &erreur);
		break;
		case MY_AUDACIOUS :
			g_spawn_command_line_async ("audacious -t", &erreur);
		break;
		case MY_BANSHEE :
			g_spawn_command_line_async ("banshee --toggle-playing", &erreur);
		break;
		case MY_EXAILE :
			g_spawn_command_line_async ("exaile -t", &erreur);
		break;
		default :
		return ;
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'play pause on %d' : %s", myConfig.iPlayer, erreur->message);
		g_error_free (erreur);
	}
}
void cd_xmms_s() {
	GError *erreur = NULL;
	switch (myConfig.iPlayer) {
		case MY_XMMS :
			g_spawn_command_line_async ("xmms -s", &erreur);
		break;
		case MY_AUDACIOUS :
			g_spawn_command_line_async ("audacious -s", &erreur);
		break;
		case MY_EXAILE :
			g_spawn_command_line_async ("exaile -s", &erreur);
		break;
		default :  // banshee n'a pas de --stop.
		return ;
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'stop on %d' : %s", myConfig.iPlayer, erreur->message);
		g_error_free (erreur);
	}
}
void cd_xmms_next() {
	GError *erreur = NULL;
	g_free (myData.playingTitle);
	myData.playingTitle = NULL; //Resetting the title to detect it for sure
	switch (myConfig.iPlayer) {
		case MY_XMMS :
			g_spawn_command_line_async ("xmms -f", &erreur);
		break;
		case MY_AUDACIOUS :
			g_spawn_command_line_async ("audacious -f", &erreur);
		break;
		case MY_BANSHEE :
			g_spawn_command_line_async ("banshee --next", &erreur);
		break;
		case MY_EXAILE :
			g_spawn_command_line_async ("exaile -n", &erreur);
		break;
		default :
		return ;
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'next on %d' : %s", myConfig.iPlayer, erreur->message);
		g_error_free (erreur);
	}
}
void cd_xmms_shuffle() {
	GError *erreur = NULL;
	switch (myConfig.iPlayer) {
		case MY_XMMS :
			g_spawn_command_line_async ("xmms -S", &erreur);
		break;
		case MY_AUDACIOUS :
			g_spawn_command_line_async ("audtool playlist-repeat-toggle ", &erreur);
		break;
		default :
		return ;
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'shuffle on %d' : %s", myConfig.iPlayer, erreur->message);
		g_error_free (erreur);
	}
}
void cd_xmms_repeat() {
	GError *erreur = NULL;
	switch (myConfig.iPlayer) {
		case MY_XMMS :
			g_spawn_command_line_async ("xmms -R", &erreur);
		break;
		case MY_AUDACIOUS :
			g_spawn_command_line_async ("audtool playlist-shuffle-toggle", &erreur);
		break;
		default :
		return ;
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'repeat on %d' : %s", myConfig.iPlayer, erreur->message);
		g_error_free (erreur);
	}
}
void cd_xmms_jumpbox() {
	GError *erreur = NULL;
	switch (myConfig.iPlayer) {
		case MY_XMMS :
			g_spawn_command_line_async ("xmms -j", &erreur);
		break;
		case MY_AUDACIOUS :
			g_spawn_command_line_async ("audacious -j", &erreur);
		break;
		default :
		return ;
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'jumpbox on %d' : %s", myConfig.iPlayer, erreur->message);
		g_error_free (erreur);
	}
}
void cd_xmms_enqueue(gchar *cFile) {
	GError *erreur = NULL;
	gchar *cCommand = NULL;
	switch (myConfig.iPlayer) {
		case MY_XMMS :
			cCommand = g_strdup_printf ("xmms -e %s", cFile);
		break;
		case MY_AUDACIOUS :
			cCommand = g_strdup_printf ("audacious -e %s", cFile);
		break;
		case MY_BANSHEE :
			cCommand = g_strdup_printf ("banshee --enqueue %s", cFile);
		break;
		case MY_EXAILE :
			//Vraiment limité ce player...
		break;
		default :
		return ;
	}
	if (cCommand != NULL && cFile != NULL) {
		cd_debug("XMMS: will use '%s'", cCommand);
		g_spawn_command_line_async (cCommand, &erreur);
		g_free(cCommand);
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'next on %d' : %s", myConfig.iPlayer, erreur->message);
		g_error_free (erreur);
	}
}
CD_APPLET_ABOUT (D_("This is the xmms applet\n made by ChAnGFu for Cairo-Dock"))

static void _xmms_action_by_id (int iAction) {
	switch (iAction) {
		case 0:
			cd_xmms_prev();
		break;
		case 1:
			cd_xmms_pp();
		break;
		case 2:
			cd_xmms_s();
		break;
		case 3:
			cd_xmms_next();
		break;
		default :
			cd_warning ("no action defined");
		break;
	}
}

CD_APPLET_ON_CLICK_BEGIN
	if (myDesklet != NULL && pClickedContainer == myContainer && pClickedIcon != NULL && pClickedIcon != myIcon) {  // clic sur une des icones du desklet.
		_xmms_action_by_id (pClickedIcon->iType);
	}
	else {
	  cd_xmms_pp();
	}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("XMMS", pSubMenu, CD_APPLET_MY_MENU)
	CD_APPLET_ADD_IN_MENU (D_("Previous"), cd_xmms_prev, pSubMenu)
	CD_APPLET_ADD_IN_MENU (D_("Play/Pause"), cd_xmms_pp, pSubMenu)
	if (myConfig.iPlayer != MY_BANSHEE) {
		CD_APPLET_ADD_IN_MENU (D_("Stop"), cd_xmms_s, pSubMenu)
	}
	CD_APPLET_ADD_IN_MENU (D_("Next"), cd_xmms_next, pSubMenu)
	if ((myConfig.iPlayer != MY_BANSHEE) && (myConfig.iPlayer != MY_EXAILE)) {
		CD_APPLET_ADD_IN_MENU (D_("Show JumpBox"), cd_xmms_jumpbox, pSubMenu)
		CD_APPLET_ADD_SUB_MENU (D_("Options"), pOpsSubMenu, pSubMenu)
		CD_APPLET_ADD_IN_MENU (D_("Toggle Shuffle"), cd_xmms_shuffle, pOpsSubMenu)
		CD_APPLET_ADD_IN_MENU (D_("Toggle Repeat"), cd_xmms_repeat, pOpsSubMenu)
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
  cd_xmms_next();
CD_APPLET_ON_MIDDLE_CLICK_END

CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message (" XMMS: %s to enqueue", CD_APPLET_RECEIVED_DATA);
	gchar *cName=NULL, *cURI=NULL, *cIconName=NULL;
	gboolean bIsDirectory;
	int iVolumeID = 0;
	double fOrder;
	/**if (cairo_dock_fm_get_file_info (CD_APPLET_RECEIVED_DATA, &cName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, 0)) {
		//if (iVolumeID == 0) {
			cd_xmms_enqueue (cURI);
		//}
	}*/
	cd_xmms_enqueue (CD_APPLET_RECEIVED_DATA);
	g_free (cName);
	g_free (cURI);
	g_free (cIconName);
CD_APPLET_ON_DROP_DATA_END
