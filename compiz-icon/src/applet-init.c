/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@hollowproject.org)

******************************************************************************/

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-load-icon.h"
#include "applet-compiz.h"

CD_APPLET_DEFINITION ("compiz-icon", 1, 5, 4, CAIRO_DOCK_CATEGORY_ACCESSORY)


CD_APPLET_INIT_BEGIN (erreur)

  myIcon->acFileName = g_strdup_printf ("%s/broken.png", MY_APPLET_SHARE_DATA_DIR);
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName)
	
	myData.iCompizIcon = COMPIZ_BROKEN;
	myData.bAcquisitionOK = FALSE;
	cd_compiz_launch_measure();
	myData.iTimer = g_timeout_add (10000, (GSourceFunc) cd_compiz_timer, (gpointer) NULL);
	
	if (myConfig.forceConfig) {
	  g_timeout_add (10000, (GSourceFunc) cd_compiz_start_wm, (gpointer) NULL);
	}
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	if (myData.iTimer != 0) {
	  g_source_remove(myData.iTimer);
	}
	
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED) {
	  myData.bNeedRedraw = TRUE;
		g_timeout_add (500, (GSourceFunc) cd_compiz_start_wm, (gpointer) NULL);
		_cd_compiz_check_for_redraw();
		cd_compiz_launch_measure();
	}
  else if (myDesklet != NULL) {
		gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
		cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, pConfig);
	}
	else {
		//Kedal
	}
CD_APPLET_RELOAD_END
