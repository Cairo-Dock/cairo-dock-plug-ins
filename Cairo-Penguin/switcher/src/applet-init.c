
#include "stdlib.h"
#include "string.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-init.h"
#include "applet-load-icons.h"

AppletConfig myConfig;
AppletData myData;


CD_APPLET_DEFINITION ("Switcher", 1, 4, 7, CAIRO_DOCK_CATEGORY_DESKTOP)


CD_APPLET_INIT_BEGIN (erreur)

	if (myDesklet != NULL) {
		myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		myIcon->fAlpha = 0.02;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	if (myIcon->acName == NULL || *myIcon->acName == '\0')
		myIcon->acName = g_strdup (SWITCHER_DEFAULT_NAME);
	
	//\_______________ On charge les icones dans un sous-dock.
myData.loadaftercompiz = g_timeout_add (2000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);
	//cd_switcher_launch_measure ();  // asynchrone
	CD_APPLET_REGISTER_FOR_CLICK_EVENT
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT

CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT
	
	g_source_remove (myData.loadaftercompiz);
	myData.loadaftercompiz = 0;
	
	//\_________________ On libere toutes nos ressources.
	reset_data ();
	reset_config ();
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
//\_______________ On recharge les donnees qui ont pu changer.

	if (myDesklet != NULL) {
		myIcon->fWidth = MAX (1, myDesklet->iWidth - g_iDockRadius);
		myIcon->fHeight = MAX (1, myDesklet->iHeight - g_iDockRadius);
		myIcon->fDrawX = g_iDockRadius/2;
		myIcon->fDrawY = g_iDockRadius/2;
		myIcon->fScale = 1;
		myIcon->fAlpha = 0.02;
		cairo_dock_load_one_icon_from_scratch (myIcon, myContainer);
		myDrawContext = cairo_create (myIcon->pIconBuffer);
		myDesklet->renderer = NULL;
	}
	if (myIcon->acName == NULL || *myIcon->acName == '\0')
		myIcon->acName = g_strdup (SWITCHER_DEFAULT_NAME);
	
	//\_______________ On charge les icones dans un sous-dock.
myData.loadaftercompiz = g_timeout_add (2000, (GSourceFunc) cd_switcher_launch_measure, (gpointer) NULL);
CD_APPLET_RELOAD_END
