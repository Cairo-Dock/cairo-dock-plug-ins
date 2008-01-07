#include <stdlib.h>
#include <glib/gi18n.h>

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
	CD_APPLET_ADD_ABOUT_IN_MENU (CD_APPLET_MY_MENU)
CD_APPLET_ON_BUILD_MENU_END
