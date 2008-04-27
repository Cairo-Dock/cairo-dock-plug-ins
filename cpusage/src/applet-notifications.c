#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-cpusage.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the cpusage applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		/// afficher : nbre de CPUs, utilisation de chacun, nbre de processus en cours, eventuellement les 3 plus gourmands (top).
	}
	else
		cairo_dock_show_temporary_dialog(D_("Acquisition of data failed"), myIcon, myContainer, 4e3);
CD_APPLET_ON_CLICK_END


static void _cpusage_recheck_ (GtkMenuItem *menu_item, gpointer *data) {
	cairo_dock_stop_measure_timer (myData.pMeasureTimer);
	cairo_dock_launch_measure (myData.pMeasureTimer);
}
static void _cpusage_change_mode_debug_ () {
	myData.inDebug = ! myData.inDebug;
}

CD_APPLET_ON_BUILD_MENU_BEGIN
		CD_APPLET_ADD_SUB_MENU ("cpusage", pSubMenu, CD_APPLET_MY_MENU)
	  	CD_APPLET_ADD_IN_MENU (D_("Debug"), _cpusage_change_mode_debug_, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
