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
#include "applet-transitions.h"
#include "applet-slider.h"

CD_APPLET_ABOUT (D_("This is the Slider applet\n made by ChAnGFu for Cairo-Dock"))

static void _cd_slider_toogle_pause(GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet) {
	//cd_message("Toggeling pause: %d", myData.bPause);
	if (!myData.bPause) {
		myData.bPause = TRUE;  // coupera le timer.
	}
	else {
		myData.bPause = FALSE;
		cd_slider_next_slide(myApplet); //on relance le diapo
	}
}

static void _cd_slider_open_current_slide (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet) {
	if (myData.pElement != NULL && myData.pElement->data != NULL) {
		SliderImage *pImage = myData.pElement->data;
		gchar *cImagePath = pImage->cPath;
		cd_debug ("opening %s ...", cImagePath);
		/*GError *erreur = NULL;
		gchar *cURI = g_filename_to_uri (cImagePath, NULL, &erreur);
		if (erreur != NULL) {
			cd_warning ("Slider : %s", erreur->message);
			g_error_free (erreur);
			return ;
		}*/
		cairo_dock_fm_launch_uri (cImagePath);  /// proposer un editeur d'image dans le panneau de conf ou une liste dans le menu...
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	switch (myConfig.iClickOption) {
		case SLIDER_PAUSE: default:
			_cd_slider_toogle_pause(NULL, myApplet);
		break;
		case SLIDER_OPEN_IMAGE:
			_cd_slider_open_current_slide(NULL, myApplet);
		break;
	}
CD_APPLET_ON_CLICK_END

static void _cd_slider_run_dir(GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet) {
	/*GError *erreur = NULL;
	gchar *cURI = g_filename_to_uri (myConfig.cDirectory, NULL, &erreur);
	if (erreur != NULL) {
		cd_warning ("Slider : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}*/
	//cd_debug("Slider: will show '%s'", cURI);
	cairo_dock_fm_launch_uri(myConfig.cDirectory);
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_cd_slider_run_dir(NULL, myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END

static void _cd_slider_previous_slide(CairoDockModuleInstance *myApplet) {
	myData.pElement = cairo_dock_get_previous_element (myData.pElement, myData.pList);
	myData.pElement = cairo_dock_get_previous_element (myData.pElement, myData.pList);
	cd_slider_next_slide(myApplet);
}


static gboolean _cd_slider_scroll_delayed (CairoDockModuleInstance *myApplet)
{
	if (myData.iNbScroll == 0)
		return FALSE;
	
	if (myConfig.bUseThread)
		cairo_dock_stop_measure_timer (myData.pMeasureImage);
	
	int i;
	if (myData.iNbScroll > 0)
	{
		if (myData.iTimerID == 0)  // en cours d'animation, on la finit en affichant l'image courante.
		{
			cd_slider_draw_default (myApplet);
			CD_APPLET_REDRAW_MY_ICON;
		}
		else
		{
			g_source_remove (myData.iTimerID);  //on coupe le timer en cours
			myData.iTimerID = 0;
		}
		for (i = 0; i < myData.iNbScroll-1; i ++)  // le 1er scroll n'implique rien.
		{
			if (myData.pElement == NULL)  // debut
				myData.pElement = myData.pList;
			else
				myData.pElement = cairo_dock_get_next_element (myData.pElement, myData.pList);
		}
	}
	else if (myData.iNbScroll < 0)
	{
		if (myData.iTimerID != 0)
		{
			g_source_remove(myData.iTimerID); //on coupe le timer en cours
			myData.iTimerID = 0;
		}
		for (i = 0; i <= -myData.iNbScroll; i ++)  // fait une fois de plus.
		{
			myData.pElement = cairo_dock_get_previous_element (myData.pElement, myData.pList);
		}
	}
	
	myData.iNbScroll = 0;
	myData.iScrollID = 0;
	cd_slider_next_slide (myApplet);
	return FALSE;
}
CD_APPLET_ON_SCROLL_BEGIN
	if (myData.iScrollID != 0)
		g_source_remove (myData.iScrollID);
	
	if (CD_APPLET_SCROLL_DOWN)
	{
		myData.iNbScroll ++;
	}
	else if (CD_APPLET_SCROLL_UP)
	{
		myData.iNbScroll --;
	}
	
	myData.iScrollID = g_timeout_add (100, (GSourceFunc) _cd_slider_scroll_delayed, myApplet);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	/**if (myConfig.bUseThread)
		cairo_dock_stop_measure_timer (myData.pMeasureImage);
	if (CD_APPLET_SCROLL_DOWN) {
		if (myData.iTimerID == 0)  // en cours d'animation, on la finit en affichant l'image courante.
		{
			cd_slider_draw_default (myApplet);
			CD_APPLET_REDRAW_MY_ICON;
		}
		else
		{
			g_source_remove(myData.iTimerID); //on coupe le timer en cours
			myData.iTimerID = 0;
		}
		cd_slider_next_slide (myApplet);  // on passe a la suivante.
	}
	else if (CD_APPLET_SCROLL_UP) {
		if (myData.iTimerID != 0)
		{
			g_source_remove(myData.iTimerID); //on coupe le timer en cours
			myData.iTimerID = 0;
		}
		_cd_slider_previous_slide(myApplet);
	}
	else
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;*/
CD_APPLET_ON_SCROLL_END

//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Slider", pSubMenu, CD_APPLET_MY_MENU);
	
		if (myConfig.iClickOption != SLIDER_PAUSE) {
			CD_APPLET_ADD_IN_MENU (myData.bPause ? D_("Play") : D_("Pause"), _cd_slider_toogle_pause, pSubMenu);
		}
		if (myConfig.iClickOption != SLIDER_OPEN_IMAGE) {
			CD_APPLET_ADD_IN_MENU (D_("Open current image"), _cd_slider_open_current_slide, pSubMenu);
		}
		CD_APPLET_ADD_IN_MENU (D_("Browse images folder"), _cd_slider_run_dir, pSubMenu);
		
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_UPDATE_ICON_BEGIN
	if (cd_slider_next_slide_is_scheduled (myApplet) || cairo_dock_measure_is_running (myData.pMeasureImage))  // on est en attente d'une image, on quitte la boucle tout de suite.
		CD_APPLET_STOP_UPDATE_ICON;
	
	gboolean bContinueTransition = FALSE;
	switch (myData.iAnimation)
	{
		case SLIDER_FADE :
			bContinueTransition = cd_slider_fade (myApplet);
		break ;
		case SLIDER_BLANK_FADE :
			bContinueTransition = cd_slider_blank_fade (myApplet);
		break ;
		case SLIDER_FADE_IN_OUT :
			bContinueTransition = cd_slider_fade_in_out (myApplet);
		break ;
		case SLIDER_SIDE_KICK :
			bContinueTransition = cd_slider_side_kick (myApplet);
		break ;
		case SLIDER_DIAPORAMA :
			bContinueTransition = cd_slider_diaporama (myApplet);
		break ;
		case SLIDER_GROW_UP :
			bContinueTransition = cd_slider_grow_up (myApplet);
		break ;
		case SLIDER_SHRINK_DOWN :
			bContinueTransition = cd_slider_shrink_down (myApplet);
		break ;
		case SLIDER_CUBE :
			bContinueTransition = cd_slider_cube (myApplet);
		break ;
		default :
			CD_APPLET_STOP_UPDATE_ICON;
	}
	
	if (! bContinueTransition)  // la transition est finie, on dessine une derniere fois, on se place en attente de l'image suivante et on quitte la boucle.
	{
		cd_slider_schedule_next_slide (myApplet);
		CD_APPLET_PAUSE_UPDATE_ICON;
	}
CD_APPLET_ON_UPDATE_ICON_END
