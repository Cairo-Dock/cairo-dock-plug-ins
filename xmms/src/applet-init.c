/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)
Fabrice Rey (fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-draw.h"
#include "applet-infopipe.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("xmms",
	1, 6, 2,
	CAIRO_DOCK_CATEGORY_CONTROLER,
	N_("An applet dedicated to control XMMS, Audacious, Banshee & Exaile.\n"
	"XMMS & Audacious are fully supported,\n"
	"but Banshee and Exaile's monitoring is still experimental.\n"
	"For XMMS, you have to install the 'xmms-infopipe' plug-in.\n"
	"Play/Pause on left click, Next song on middle click.\n"
	"You can drag and drop songs to put them in the queue.\n"
	"Scroll updown to play the previous/next sound\n"
	"Down play the next sound"),
	"ChAnGFu (Rémy Robertson)")

static gchar *s_cPlayerClass[MY_NB_PLAYERS] = {"xmms", "audacious", "banshee", "exaile"};

CD_APPLET_INIT_BEGIN
	if (myDesklet) {
		if (myConfig.extendedDesklet) {
			cd_xmms_add_buttons_to_desklet (myApplet);
			if (myConfig.iExtendedMode == MY_DESKLET_INFO || myConfig.iExtendedMode == MY_DESKLET_INFO_AND_CONTROLER) {
				gpointer data[3] = {"None", "None", GINT_TO_POINTER (myConfig.iExtendedMode == MY_DESKLET_INFO ? FALSE : TRUE)};
				CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Mediaplayer", data);
			}
			else {
				gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
				CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", data);
			}
		}
		else {
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		}
	}
	
	myData.playingStatus = PLAYER_NONE;
	myData.previousPlayingStatus = -1;
	myData.previousPlayingTitle = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	myData.pMeasureTimer = cairo_dock_new_measure_timer (1,
		(CairoDockAquisitionTimerFunc) cd_xmms_acquisition,
		(CairoDockReadTimerFunc) cd_xmms_read_data,
		(CairoDockUpdateTimerFunc) cd_xmms_draw_icon,
		myApplet);
	cairo_dock_launch_measure (myData.pMeasureTimer);
	
	if (myConfig.bStealTaskBarIcon) {
		cairo_dock_inhibate_class (s_cPlayerClass[myConfig.iPlayer], myIcon);
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_DROP_DATA_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	
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
			cd_xmms_add_buttons_to_desklet (myApplet);
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
			if (myConfig.iExtendedMode == MY_DESKLET_INFO || myConfig.iExtendedMode == MY_DESKLET_INFO_AND_CONTROLER) {
				gpointer data[3] = {"None", "None", GINT_TO_POINTER (myConfig.iExtendedMode == MY_DESKLET_INFO ? FALSE : TRUE)};
				CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Mediaplayer", data);
			}
			else {
				gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
				CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA ("Caroussel", data);
			}
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
		cd_xmms_draw_icon (myApplet);
	}
CD_APPLET_RELOAD_END
