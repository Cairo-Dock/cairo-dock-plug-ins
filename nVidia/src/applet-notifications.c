/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-draw.h"


CD_APPLET_ABOUT (D_("This is the nVidia applet\n made by ChAnGFu for Cairo-Dock"))


//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	cairo_dock_remove_dialog_if_any (myIcon);
	cd_nvidia_bubble();
	
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END

static void nvidia_setting(void) {  /// a mettre dans les plug-ins d'integration.  // je ne pense pas que ca depende de l'environnement de bureau. Ils ont un truc specifiqe sous KDE ?
	GError *erreur = NULL;
	switch (g_iDesktopEnv) {
		case CAIRO_DOCK_GNOME :
		case CAIRO_DOCK_XFCE :
			g_spawn_command_line_async ("env LC_NUMERIC=C gksu nvidia-settings", &erreur);
		break;
		case CAIRO_DOCK_KDE :
			g_spawn_command_line_async ("env LC_NUMERIC=C kdesu nvidia-settings", &erreur);
		break ;
		default :
			cd_warning ("couldn't guess system WM");
		return ;
	}
	if (erreur != NULL) {
		cd_warning ("nVidia : %s", erreur->message);
		g_error_free (erreur);
	}
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	///On ajoutera la désactivation quand elle sera fonctionnelle... // la desactivation de quoi ?
	nvidia_setting();
	cairo_dock_launch_measure (myData.pMeasureTimer);
	cairo_dock_remove_dialog_if_any (myIcon);
CD_APPLET_ON_MIDDLE_CLICK_END
