#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"

CD_APPLET_INCLUDE_MY_VARS

extern gboolean inDebug;

CD_APPLET_ABOUT (D_("This is the netspeed applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	/* cairo_dock_show_temporary_dialog("BLAH !", myIcon, myContainer, 0);*/
	if (myData.bAcquisitionOK)
		cairo_dock_show_temporary_dialog(D_("Total amount of data :\n  downloaded : %.2fKB\n  uploaded : %.2fKB"), myIcon, myContainer, 10e3, myData.iReceivedBytes/1000., myData.iTransmittedBytes/1000.);
	else
		cairo_dock_show_temporary_dialog(D_("Interface '%s' seems to not exist or is not readable"), myIcon, myContainer, 10e3, myConfig.cInterface);
CD_APPLET_ON_CLICK_END


static void _netspeed_recheck_ (GtkMenuItem *menu_item, gpointer *data) {
		if (myData.iSidTimer != 0) {
		  g_source_remove (myData.iSidTimer);
		  myData.iSidTimer = 0;
	  }
	  myConfig.iCheckInterval = myConfig.dCheckInterval;
	  //cd_netspeed_launch_analyse();
	  cairo_dock_launch_measure (myData.pMeasureTimer);
}
static void _netspeed_change_mode_debug_ () {
	inDebug = 1 - inDebug;
}

CD_APPLET_ON_BUILD_MENU_BEGIN
		CD_APPLET_ADD_SUB_MENU ("netspeed", pSubMenu, CD_APPLET_MY_MENU)
		if (! myData.bAcquisitionOK) {
	    		CD_APPLET_ADD_IN_MENU (D_("Re-check interface"), _netspeed_recheck_, pSubMenu)
	  	}
	  	CD_APPLET_ADD_IN_MENU (D_("Debug"), _netspeed_change_mode_debug_, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
