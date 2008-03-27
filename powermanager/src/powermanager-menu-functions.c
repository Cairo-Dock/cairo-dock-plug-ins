#include <stdlib.h>
#include <glib/gi18n.h>

#include "powermanager-dbus.h"
#include "powermanager-struct.h"
#include "powermanager-menu-functions.h"

CD_APPLET_INCLUDE_MY_VARS


//*********************************************************************************
// Informations sur l'applet et l'auteur.
//*********************************************************************************
CD_APPLET_ABOUT (_D("Applet by Necropotame (Adrien Pilleboue)"))

//*********************************************************************************
// Fonction appelée a la construction du menu.
// Cette fonction remplit le menu principal avec les actions previous, next, et information.
//*********************************************************************************
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (myData.dbus_enable)
	{
		CD_APPLET_ADD_IN_MENU (_D("Halt"), power_halt, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_D("Hibernate"), power_hibernate, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_D("Suspend"), power_suspend, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_D("Reboot"), power_reboot, CD_APPLET_MY_MENU)
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU)
CD_APPLET_ON_BUILD_MENU_END
