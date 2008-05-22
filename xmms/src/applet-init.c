/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-infopipe.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("xmms", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)

static gchar *s_cPlayerClass[MY_NB_PLAYERS] = {"xmms", "audacious", "banshee", "exaile"};

CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet) {
		if (myConfig.extendedDesklet) {
			cd_xmms_add_buttons_to_desklet ();
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", data);
		}
		else {
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
	}
	
	cd_xmms_remove_pipes();
	
	myData.playingStatus = PLAYER_NONE;
	myData.previousPlayingStatus = -1;
	myData.previousPlayingTitle = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	myData.pMeasureTimer = cairo_dock_new_measure_timer (1,
		cd_xmms_acquisition,
		cd_xmms_read_data,
		cd_xmms_draw_icon);
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	if (myConfig.bStealTaskBarIcon) {
		cairo_dock_inhibate_class (s_cPlayerClass[myConfig.iPlayer], myIcon);
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT
	
	cd_xmms_remove_pipes();
	
	if (myIcon->cClass != NULL)
		cairo_dock_deinhibate_class (s_cPlayerClass[myConfig.iPlayer], myIcon);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED && myDesklet) {
		if ( ! myConfig.extendedDesklet && myDesklet->icons != NULL) {
			g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myDesklet->icons);
			myDesklet->icons = NULL;
		}
		else if (myConfig.extendedDesklet && myDesklet->icons == NULL) {
			cd_xmms_add_buttons_to_desklet ();
		}
	}
	
	int i;
	for (i = 0; i < PLAYER_NB_STATUS; i ++) { // reset surfaces.
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	
	if (myDesklet) {
		if (myConfig.extendedDesklet) {
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", data);
		}
		else {
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
	}
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	myData.playingStatus = PLAYER_NONE;
	myData.previousPlayingStatus = -1;
	myData.previousPlayingTitle = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myIcon->cClass != NULL) { // on est en train d'inhiber l'appli.
			if (! myConfig.bStealTaskBarIcon || strcmp (myIcon->cClass, s_cPlayerClass[myConfig.iPlayer]) != 0) { // on ne veut plus l'inhiber ou on veut inhiber une autre.
				cairo_dock_deinhibate_class (myIcon->cClass, myIcon);
			}
		}
		if (myConfig.bStealTaskBarIcon && myIcon->cClass == NULL) { // on comence a inhiber l'appli si on ne le faisait pas, ou qu'on s'est arrete.
			cairo_dock_inhibate_class (s_cPlayerClass[myConfig.iPlayer], myIcon);
		}
		// inutile de relancer le timer, sa frequence ne change pas. Inutile aussi de faire 1 iteration ici, les modifs seront prises en compte a la prochaine iteration, dans au plus 1s.
	}
	else {  // on redessine juste l'icone.
		cd_xmms_draw_icon ();
	}
CD_APPLET_RELOAD_END
