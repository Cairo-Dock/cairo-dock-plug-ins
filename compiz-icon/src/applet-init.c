/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@hollowproject.org)
Fabrice Rey <fabounet@users.berlios.de>

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-load-icon.h"
#include "applet-compiz.h"

CD_APPLET_DEFINITION ("compiz-icon", 1, 5, 4, CAIRO_DOCK_CATEGORY_DESKTOP)


CD_APPLET_INIT_BEGIN (erreur)
	cd_compiz_build_icons ();
	
	if (myConfig.bAutoReloadDecorator || myConfig.bAutoReloadCompiz) {
		myData.iCompizIcon = -1;
		if (! myConfig.forceConfig) { // on fait comme si c'est nous qui l'avons mis dans l'etat actuel.
			myData.bCompizRestarted = TRUE;
			myData.bDecoratorRestarted = TRUE;
		}
		cd_compiz_launch_measure ();
	}
	else {
		CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_DEFAULT], "default.svg");
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	if (myData.iSidTimer != 0) {
		g_source_remove(myData.iSidTimer);
	}
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
		
		//\________________ les icones ont pu changer.
		if (myIcon->pSubDock != NULL) {
			cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->acName, NULL, NULL);
			myIcon->pSubDock = NULL;
		}
		if (myDesklet && myDesklet->icons != NULL) {
			g_list_foreach (myDesklet->icons, cairo_dock_free_icon, NULL);
			g_list_free (myDesklet->icons);
			myDesklet->icons = NULL;
		}
		cd_compiz_build_icons ();
		
		if (myData.iSidTimer != 0 && ! myConfig.bAutoReloadDecorator && ! myConfig.bAutoReloadCompiz) {
			g_source_remove(myData.iSidTimer);
			myData.iSidTimer = 0;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_DEFAULT], "default.svg");
		}
		else if (myData.iSidTimer == 0 && (myConfig.bAutoReloadDecorator || myConfig.bAutoReloadCompiz)) {
			myData.iCompizIcon = -1;
			if (! myConfig.forceConfig) { // on fait comme si c'est nous qui l'avons mis dans l'etat actuel.
				myData.bCompizRestarted = TRUE;
				myData.bDecoratorRestarted = TRUE;
			}
			cd_compiz_launch_measure ();
		}
		else {
			if (myData.iSidTimer != 0)
				myData.iCompizIcon = -1;
			CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cUserImage[COMPIZ_DEFAULT], "default.svg");
		}
		
	}
	else if (myDesklet != NULL) {
		gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, pConfig);
	}
	else {
		//Rien a faire
	}
CD_APPLET_RELOAD_END
