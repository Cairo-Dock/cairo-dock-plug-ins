/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <cairo-dock.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-transitions.h"
#include "applet-slider.h"

static inline void _toggle_pause (GldiModuleInstance *myApplet)
{
	if (!myData.bPause) {
		myData.bPause = TRUE;  // will stop the slider.
	}
	else {
		myData.bPause = FALSE;
		cd_slider_jump_to_next_slide (myApplet);  // restart the slider
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
static void _open_current_slide (GldiModuleInstance *myApplet)
{
	if (myData.pElement != NULL && myData.pElement->data != NULL)
	{
		SliderImage *pImage = myData.pElement->data;
		gchar *cImagePath = pImage->cPath;
		cd_debug ("opening %s ...", cImagePath);
		cairo_dock_fm_launch_uri (cImagePath);
	}
}
static void _open_current_folder (GldiModuleInstance *myApplet)
{
	if (myData.pElement != NULL && myData.pElement->data != NULL)
	{
		SliderImage *pImage = myData.pElement->data;
		gchar *cDirPath = g_path_get_dirname (pImage->cPath);
		cd_debug ("opening folder %s ...", cDirPath);
		cairo_dock_fm_launch_uri (cDirPath);
		g_free (cDirPath);
	}
}
static void _cd_slider_action (SliderClickOption iAction, GldiModuleInstance *myApplet)
{
	if (myConfig.cDirectory == NULL)
	{
		gldi_dialog_show_temporary_with_icon (D_("You need to define the images folder first."), myIcon, myContainer, 8000, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
		return;
	}
	switch (iAction)
	{
		case SLIDER_PAUSE:
		default:
			_toggle_pause (myApplet);
		break;
		case SLIDER_OPEN_IMAGE:
			_open_current_slide (myApplet);
		break;
		case SLIDER_OPEN_FOLDER:
			_open_current_folder (myApplet);
		break;
	}
}
CD_APPLET_ON_CLICK_BEGIN
	_cd_slider_action (myConfig.iClickOption, myApplet);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_cd_slider_action (myConfig.iMiddleClickOption, myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END


static gboolean _cd_slider_scroll_delayed (GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	if (myData.iNbScroll == 0)
	{
		myData.iScrollID = 0;
		CD_APPLET_LEAVE (FALSE);
	}
	
	int i;
	if (myData.iNbScroll > 0)
	{
		for (i = 0; i < myData.iNbScroll-1; i ++)  // N-1 because we'll jump to next slide.
		{
			if (myData.pElement == NULL)  // debut
				myData.pElement = myData.pList;
			else
				myData.pElement = cairo_dock_get_next_element (myData.pElement, myData.pList);
		}
	}
	else if (myData.iNbScroll < 0)
	{
		for (i = 0; i <= -myData.iNbScroll; i ++)  // N+1 because we'll jump to next slide.
		{
			if (myData.pElement == NULL)  // debut
				myData.pElement = myData.pList;
			else
				myData.pElement = cairo_dock_get_previous_element (myData.pElement, myData.pList);
		}
	}
	
	myData.iNbScroll = 0;
	myData.iScrollID = 0;
	cd_slider_jump_to_next_slide (myApplet);
	CD_APPLET_LEAVE (FALSE);
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
	CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
CD_APPLET_ON_SCROLL_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static void _free_app (gpointer *app)
{
	g_free (app[1]);
	g_free (app);
}
void cd_slider_free_apps_list (GldiModuleInstance *myApplet)
{
	if (myData.pAppList != NULL)
	{
		g_list_foreach (myData.pAppList, (GFunc) _free_app, NULL);
		g_list_free (myData.pAppList);
		myData.pAppList = NULL;
	}
}

static void _cd_slider_toogle_pause (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_toggle_pause (myApplet);
	CD_APPLET_LEAVE();
}
static void _cd_slider_open_selected (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cairo_dock_fm_launch_uri (myData.cSelectedImagePath);
	CD_APPLET_LEAVE();
}
static void _cd_slider_run_dir (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	if (myData.cSelectedImagePath == NULL)
	{
		cairo_dock_fm_launch_uri (myConfig.cDirectory);
	}
	else
	{
		gchar *cDirPath = g_path_get_dirname (myData.cSelectedImagePath);
		cairo_dock_fm_launch_uri (cDirPath);
		g_free (cDirPath);
	}
	CD_APPLET_LEAVE();
}
static void _cd_slider_refresh_images_list (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cd_slider_stop (myApplet);
	
	cd_slider_start (myApplet, FALSE);  // FALSE <=> immediately
	CD_APPLET_LEAVE();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	// get the current image selected at the moment of the click.
	g_free (myData.cSelectedImagePath);
	if (myData.pElement != NULL && myData.pElement->data != NULL)
	{
		SliderImage *pImage = myData.pElement->data;
		myData.cSelectedImagePath = g_strdup (pImage->cPath);
	}
	else
		myData.cSelectedImagePath = NULL;
	
	gchar *cLabel;
	if (myConfig.iClickOption != SLIDER_PAUSE)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (myData.bPause ? D_("Play") : D_("Pause"), myData.bPause ? GLDI_ICON_NAME_MEDIA_PLAY : GLDI_ICON_NAME_MEDIA_PAUSE, _cd_slider_toogle_pause, CD_APPLET_MY_MENU);
	}
	
	if (myData.cSelectedImagePath != NULL)
	{
		if (myConfig.iClickOption != SLIDER_OPEN_IMAGE)
		{
			if (myConfig.iMiddleClickOption == SLIDER_OPEN_IMAGE)
				cLabel = g_strdup_printf ("%s (%s)", D_("Open current image"), D_("middle-click"));
			else
				cLabel = g_strdup (D_("Open current image"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GLDI_ICON_NAME_OPEN, _cd_slider_open_selected, CD_APPLET_MY_MENU);
			g_free (cLabel);
		}
		
		GList *pApps = cairo_dock_fm_list_apps_for_file (myData.cSelectedImagePath);
		if (pApps != NULL)
		{
			cairo_dock_fm_add_open_with_submenu (pApps, myData.cSelectedImagePath, CD_APPLET_MY_MENU, D_("Open with"),
				GLDI_ICON_NAME_OPEN, NULL, NULL);
			g_list_free_full (pApps, g_object_unref);
		}
	}
	
	if (myConfig.iMiddleClickOption == SLIDER_OPEN_FOLDER)
		cLabel = g_strdup_printf ("%s (%s)", D_("Browse images folder"), D_("middle-click"));
	else
		cLabel = g_strdup (D_("Browse images folder"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GLDI_ICON_NAME_DIRECTORY, _cd_slider_run_dir, CD_APPLET_MY_MENU);
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Refresh images list"), GLDI_ICON_NAME_REFRESH, _cd_slider_refresh_images_list, CD_APPLET_MY_MENU);
	
	g_free (cLabel);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_UPDATE_ICON_BEGIN
	if (cd_slider_next_slide_is_scheduled (myApplet) || gldi_task_is_running (myData.pMeasureImage))  // on est en attente d'une image, on quitte la boucle tout de suite.
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
