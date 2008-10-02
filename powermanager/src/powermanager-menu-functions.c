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
CD_APPLET_ABOUT (D_("Applet by Necropotame (Adrien Pilleboue)"))

CD_APPLET_ON_CLICK_BEGIN
	cairo_dock_remove_dialog_if_any (myIcon);
	cd_powermanager_bubble();
CD_APPLET_ON_CLICK_END

void power_config(void) {  /// a mettre dans les plug-ins d'integration.
	GError *erreur = NULL;
	if (g_iDesktopEnv == CAIRO_DOCK_GNOME || g_iDesktopEnv == CAIRO_DOCK_XFCE)
	{
		g_spawn_command_line_async ("gnome-power-preferences", &erreur);
	}
	else if (g_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		//Ajouter les lignes de KDE
	}
	if (erreur != NULL)
	{
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
	}
}

//*********************************************************************************
// Fonction appelée a la construction du menu.
// Cette fonction remplit le menu principal avec les actions previous, next, et information.
//*********************************************************************************
CD_APPLET_ON_BUILD_MENU_BEGIN
	//on rajoute un sous menu, sinon ce n'est pas esthétique
	CD_APPLET_ADD_SUB_MENU ("PowerManager", pSubMenu, CD_APPLET_MY_MENU);
	CD_APPLET_ADD_IN_MENU (D_("Set up power management"), power_config, pSubMenu);
	if (myData.dbus_enable)
	{
		CD_APPLET_ADD_IN_MENU (D_("Halt"), power_halt, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Hibernate"), power_hibernate, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Suspend"), power_suspend, pSubMenu);
		CD_APPLET_ADD_IN_MENU (D_("Reboot"), power_reboot, pSubMenu);
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END
