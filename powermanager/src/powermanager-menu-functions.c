#include <stdlib.h>
#include <glib/gi18n.h>

#include "powermanager-dbus.h"
#include "powermanager-draw.h"
#include "powermanager-struct.h"
#include "powermanager-menu-functions.h"

CD_APPLET_INCLUDE_MY_VARS


//*********************************************************************************
// Informations sur l'applet et l'auteur.
//*********************************************************************************
CD_APPLET_ABOUT (_D("Applet by Necropotame (Adrien Pilleboue)"))

CD_APPLET_ON_CLICK_BEGIN
	cd_powermanager_bubble();
CD_APPLET_ON_CLICK_END

//*********************************************************************************
// Fonction appelée a la construction du menu.
// Cette fonction remplit le menu principal avec les actions previous, next, et information.
//*********************************************************************************
CD_APPLET_ON_BUILD_MENU_BEGIN
  //on rajoute un sous menu, sinon ce n'est pas esthétique
  CD_APPLET_ADD_SUB_MENU ("PowerManager", pSubMenu, CD_APPLET_MY_MENU)
	if (myData.dbus_enable)
	{
	    CD_APPLET_ADD_IN_MENU (_D("Halt"), power_halt, pSubMenu)
		  CD_APPLET_ADD_IN_MENU (_D("Hibernate"), power_hibernate, pSubMenu)
		  CD_APPLET_ADD_IN_MENU (_D("Suspend"), power_suspend, pSubMenu)
		  CD_APPLET_ADD_IN_MENU (_D("Reboot"), power_reboot, pSubMenu)
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
