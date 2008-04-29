#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-rame.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the rame applet\n made by parAdOxxx_ZeRo for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	/// montrer les 3 applis les plus gourmandes + tous les parametres qu'on recupere.
CD_APPLET_ON_CLICK_END


static void _rame_recheck_ (GtkMenuItem *menu_item, gpointer *data) {
	cairo_dock_stop_measure_timer (myData.pMeasureTimer);
	cairo_dock_launch_measure (myData.pMeasureTimer);
}

CD_APPLET_ON_BUILD_MENU_BEGIN
		CD_APPLET_ADD_SUB_MENU ("rame", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
