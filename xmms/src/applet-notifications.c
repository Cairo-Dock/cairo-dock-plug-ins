#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-infopipe.h"

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;

void cd_xmms_prev() {
	GError *erreur = NULL;
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
		cd_warning ("Attention : when trying to execute 'xmms -j' : %s", erreur->message);
		g_error_free (erreur);
	}
}

CD_APPLET_ABOUT (_D("This is the xmms applet\n made by ChAnGFu for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	cd_xmms_pp();
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("XMMS", pSubMenu, CD_APPLET_MY_MENU)
	CD_APPLET_ADD_IN_MENU (_D("Previous"), cd_xmms_prev, pSubMenu)
	CD_APPLET_ADD_IN_MENU (_D("Play/Pause"), cd_xmms_pp, pSubMenu)
	if (myConfig.iPlayer != MY_BANSHEE) {
		CD_APPLET_ADD_IN_MENU (_D("Stop"), cd_xmms_s, pSubMenu)
	}
	CD_APPLET_ADD_IN_MENU (_D("Next"), cd_xmms_next, pSubMenu)
	if ((myConfig.iPlayer != MY_BANSHEE) && (myConfig.iPlayer != MY_EXAILE)) {
		CD_APPLET_ADD_IN_MENU (_D("Show JumpBox"), cd_xmms_jumpbox, pSubMenu)
		CD_APPLET_ADD_SUB_MENU (_D("Options"), pOpsSubMenu, pSubMenu)
		CD_APPLET_ADD_IN_MENU (_D("Toggle Shuffle"), cd_xmms_shuffle, pOpsSubMenu)
		CD_APPLET_ADD_IN_MENU (_D("Toggle Repeat"), cd_xmms_repeat, pOpsSubMenu)
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
  cd_xmms_next();
CD_APPLET_ON_MIDDLE_CLICK_END
