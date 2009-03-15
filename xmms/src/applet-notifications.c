/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-infopipe.h"


void cd_xmms_prev(CairoDockModuleInstance *myApplet) {
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

void cd_xmms_pp(CairoDockModuleInstance *myApplet) {
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
void cd_xmms_stop(CairoDockModuleInstance *myApplet) {
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
void cd_xmms_next(CairoDockModuleInstance *myApplet) {
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
void cd_xmms_shuffle(CairoDockModuleInstance *myApplet) {
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
void cd_xmms_repeat(CairoDockModuleInstance *myApplet) {
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
void cd_xmms_jumpbox(CairoDockModuleInstance *myApplet) {
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
void cd_xmms_enqueue(CairoDockModuleInstance *myApplet, const gchar *cFile) {
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


static void _xmms_action_by_id (CairoDockModuleInstance *myApplet, int iAction) {
	switch (iAction) {
		case 0:
			cd_xmms_prev(myApplet);
		break;
		case 1:
			cd_xmms_pp(myApplet);
		break;
		case 2:
			cd_xmms_stop(myApplet);
		break;
		case 3:
			cd_xmms_next(myApplet);
		break;
		default :
			cd_warning ("no action defined");
		break;
	}
}
CD_APPLET_ON_CLICK_BEGIN
	if (myDesklet != NULL && pClickedContainer == myContainer && pClickedIcon != NULL && pClickedIcon != myIcon) {  // clic sur une des icones du desklet.
		_xmms_action_by_id (myApplet, pClickedIcon->iType);
	}
	else {
	  cd_xmms_pp(myApplet);
	}
CD_APPLET_ON_CLICK_END


static void _on_play_pause (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_xmms_pp (myApplet);
}
static void _on_prev (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_xmms_prev (myApplet);
}
static void _on_next (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_xmms_next (myApplet);
}
static void _on_stop (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_xmms_stop (myApplet);
}
static void _on_jumpbox (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_xmms_jumpbox (myApplet);
}
static void _on_shuffle (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_xmms_shuffle (myApplet);
}
static void _on_repeat (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cd_xmms_repeat (myApplet);
}

CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
	CD_APPLET_ADD_IN_MENU (D_("Previous"), _on_prev, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU (D_("Play/Pause (left-click)"), _on_play_pause, CD_APPLET_MY_MENU);
	if (myConfig.iPlayer != MY_BANSHEE) {
		CD_APPLET_ADD_IN_MENU (D_("Stop"), _on_stop, CD_APPLET_MY_MENU);
	}
	CD_APPLET_ADD_IN_MENU (D_("Next (middle-click)"), _on_next, CD_APPLET_MY_MENU);
	if ((myConfig.iPlayer != MY_BANSHEE) && (myConfig.iPlayer != MY_EXAILE)) {
		CD_APPLET_ADD_IN_MENU (D_("Show JumpBox"), _on_jumpbox, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Toggle Shuffle"), _on_shuffle, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Toggle Repeat"), _on_repeat, pSubMenu);
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	cd_xmms_next(myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_DROP_DATA_BEGIN
	cd_message (" XMMS: %s to enqueue", CD_APPLET_RECEIVED_DATA);
	cd_xmms_enqueue (myApplet, CD_APPLET_RECEIVED_DATA);
CD_APPLET_ON_DROP_DATA_END



CD_APPLET_ON_SCROLL_BEGIN
		if (CD_APPLET_SCROLL_DOWN) {
			cd_xmms_next(myApplet);
		}
		else if (CD_APPLET_SCROLL_UP) {
			cd_xmms_prev(myApplet);
		}
		else
			return CAIRO_DOCK_LET_PASS_NOTIFICATION;
CD_APPLET_ON_SCROLL_END
