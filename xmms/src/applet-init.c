#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-infopipe.h"
#include "applet-init.h"


CD_APPLET_DEFINITION ("xmms", 1, 5, 4, CAIRO_DOCK_CATEGORY_CONTROLER)

static gchar *s_cPlayerClass[MY_NB_PLAYERS] = {"xmms", "audacious", "banshee", "exaile"};

static gchar *s_cControlIconName[4] = {"play.svg", "pause.svg", "stop.svg", "broken.svg"};  // en attendant...
#define _add_icon(i)\
	pIcon = g_new0 (Icon, 1);\
	pIcon->acName = NULL;\
	pIcon->acFileName = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, s_cControlIconName[i]);\
	pIcon->fOrder = i;\
	pIcon->fScale = 1.;\
	pIcon->fAlpha = 1.;\
	pIcon->fWidthFactor = 1.;\
	pIcon->fHeightFactor = 1.;\
	pIcon->acCommand = g_strdup ("none");\
	pIcon->cParentDockName = NULL;\
	myDesklet->icons = g_list_append (myDesklet->icons, pIcon);

CD_APPLET_INIT_BEGIN (erreur)
	if (myDesklet) {
		if (myConfig.extendedDesklet)
		{
			int i;
			for (i = 0; i < 4; i ++)
			{
				_add_icon(i);
			}
			g_print ("mode etendu\n");
			gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
			cairo_dock_set_desklet_renderer_by_name (myDesklet, "Controler", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, data);
		}
		else
			cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	cd_remove_pipes();
	
	myData.playingStatus = PLAYER_NONE;
	myData.previousPlayingStatus = -1;
	myData.previousPlayingTitle = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	cd_xmms_launch_measure ();
	
	if (myConfig.bStealTaskBarIcon)
	{
		cairo_dock_inhibate_class (s_cPlayerClass[myConfig.iPlayer], myIcon);
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	cd_remove_pipes();
	
	if (myIcon->cClass != NULL)
		cairo_dock_deinhibate_class (s_cPlayerClass[myConfig.iPlayer], myIcon);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED && CD_APPLET_MY_CONTAINER_TYPE_CHANGED && myDesklet && ! myConfig.extendedDesklet) {
		g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
		g_list_free (myDesklet->icons);
		myDesklet->icons = NULL;
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
			g_print ("mode etendu\n");
			cd_xmms_add_button_to_desklet(); //on dessine le desklet avec ses boutons
		}
		else
			cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
	}
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	myData.playingStatus = PLAYER_NONE;
	myData.previousPlayingStatus = -1;
	myData.previousPlayingTitle = NULL;
	myData.iPreviousTrackNumber = -1;
	myData.iPreviousCurrentTime = -1;
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		if (myIcon->cClass != NULL)  // on est en trian d'inhiber l'appli.
		{
			if (! myConfig.bStealTaskBarIcon || strcmp (myIcon->cClass, s_cPlayerClass[myConfig.iPlayer]) != 0)  // on ne veut plus l'inhiber ou on veut inhiber une autre.
			{
				cairo_dock_deinhibate_class (myIcon->cClass, myIcon);
			}
		}
		if (myConfig.bStealTaskBarIcon && myIcon->cClass == NULL)  // on comence a inhiber l'appli si on ne le faisait pas, ou qu'on s'est arrete.
		{
			cairo_dock_inhibate_class (s_cPlayerClass[myConfig.iPlayer], myIcon);
		}
		// inutile de relancer le timer, sa frequence ne change pas. Inutile aussi de faire 1 iteration ici, les modifs seront prises en compte a la prochaine iteration, dans au plus 1s.
	}
	else {  // on redessine juste l'icone.
		cd_xmms_draw_icon ();
	}
CD_APPLET_RELOAD_END
