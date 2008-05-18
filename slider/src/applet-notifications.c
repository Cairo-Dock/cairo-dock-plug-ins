/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <cairo-dock.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-silder.h"

CD_APPLET_INCLUDE_MY_VARS

CD_APPLET_ABOUT (D_("This is the Slider applet\n made by ChAnGFu for Cairo-Dock"))

static void _cd_slider_toogle_pause(void) {
	cd_message("Toggeling pause: %d", myData.bPause);
	if (!myData.bPause) {
		myData.bPause = TRUE;
		if (myData.iTimerID != 0) {
			g_source_remove(myData.iTimerID); //on coupe le timer en cours
			myData.iTimerID = 0;
		}
	}
	else {
		myData.bPause = FALSE;
		cd_slider_draw_images(); //on relance le diapo
	}
}

static void _cd_slider_open_current_slide (void) {
	if (myData.pElement != NULL && myData.pElement->data != NULL) {
		gchar *cImagePath = myData.cCurrentImagePath;
		GError *erreur = NULL;
		gchar *cURI = g_filename_to_uri (cImagePath, NULL, &erreur);
		cd_debug ("Now opening image URI:%s Filename:%s", cURI, cImagePath);
		if (erreur != NULL) {
			cd_warning ("Attention : %s", erreur->message);
			g_error_free (erreur);
			return ;
		}
		cairo_dock_fm_launch_uri (cURI);  /// proposer un editeur d'image dans le panneau de conf.
		g_free (cURI);
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	switch (myConfig.iClickOption) {
		case SLIDER_PAUSE: default:
			_cd_slider_toogle_pause();
		break;
		case SLIDER_OPEN_IMAGE:
			_cd_slider_open_current_slide();
		break;
	}
CD_APPLET_ON_CLICK_END

static void _cd_slider_run_dir(void) {
	GError *erreur = NULL;
	gchar *cURI = g_filename_to_uri (myConfig.cDirectory, NULL, &erreur);
	if (erreur != NULL) {
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
	cd_debug("Slider: will show '%s'", cURI);
	cairo_dock_fm_launch_uri(cURI);
	g_free (cURI);
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_cd_slider_run_dir();
CD_APPLET_ON_MIDDLE_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Slider", pSubMenu, CD_APPLET_MY_MENU)
	
		if (myConfig.iClickOption != SLIDER_PAUSE) {
			CD_APPLET_ADD_IN_MENU (myData.bPause ? D_("Play") : D_("Pause"), _cd_slider_toogle_pause, pSubMenu)
		}
		if (myConfig.iClickOption != SLIDER_OPEN_IMAGE) {
			CD_APPLET_ADD_IN_MENU (D_("Open current image"), _cd_slider_open_current_slide, pSubMenu)
		}
		/// ajouter _cd_slider_run_dir aussi ?
		
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
